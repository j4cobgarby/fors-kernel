ARCH = x64
BUILD = build
KERN_DIR = kernel
SRC = $(KERN_DIR)/src
TARGET_ISO = fors.iso

QEMU_DISPLAY_TYPE = gtk

KERNEL_EXE = $(BUILD)/fors.elf

CC = clang
LD = ld

CFLAGS = -Wall -Wextra  	\
	-std=c2x			 	\
	-nostdlib               \
	-ffreestanding		 	\
	-fno-stack-protector 	\
	-fno-stack-check     	\
	-fno-lto             	\
	-fno-pie             	\
	-fno-pic             	\
	-m64                 	\
	-march=x86-64        	\
	-mabi=sysv           	\
	-mno-80387           	\
	-mno-mmx             	\
	-mno-sse             	\
	-mno-sse2            	\
	-mno-red-zone        	\
	-mcmodel=kernel      	\
	-I$(KERN_DIR)                  	\
	-I$(SRC)/include           \
	-Wno-unused-parameter	\
	-Wno-unused-function 	\
	-D__ARCH_$(ARCH)__   	\
	-ggdb                   \

LDFLAGS = \
	-nostdlib -static \
	-z max-page-size=0x1000 \
	-T $(KERN_DIR)/linker.ld \
	-m elf_x86_64 \
	-no-pie

NASMFLAGS += \
    -f elf64 -g

CFILES = $(shell find $(SRC)/generic -type f -name '*.c') $(shell find $(SRC)/arch/$(ARCH) -type f -name '*.c')
NASMFILES := $(shell find $(SRC)/generic -type f -name '*.asm') $(shell find $(SRC)/arch/$(ARCH) -type f -name '*.asm')
OBJS = $(addprefix $(BUILD)/, $(CFILES:.c=.o) $(NASMFILES:.asm=.o))

.PHONY: all
all: $(TARGET_ISO)

$(TARGET_ISO): $(KERNEL_EXE)
	@rm -rf $(BUILD)/iso
	@mkdir -p $(BUILD)/iso

	@cp $(KERNEL_EXE) \
		limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin \
		limine/limine-uefi-cd.bin \
		$(BUILD)/iso
	@mkdir -p $(BUILD)/iso/EFI/BOOT
	@cp limine/BOOTX64.EFI limine/BOOTIA32.EFI $(BUILD)/iso/EFI/BOOT/

	@echo [XORRISO] Creating disk image $@
	@xorriso -as mkisofs -b limine-bios-cd.bin -no-emul-boot \
		-boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin -efi-boot-part \
		--efi-boot-image --protective-msdos-label \
		$(BUILD)/iso -o $@ 2>/dev/null
	@./limine/limine bios-install $@ 2>/dev/null

$(KERNEL_EXE): $(OBJS)
	@$(LD) $(OBJS) $(LDFLAGS) -o $@

$(BUILD)/$(SRC)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	@echo [CC] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(SRC)/%.o: $(SRC)/%.asm
	@mkdir -p $(dir $@)
	@echo [NASM] $<
	@nasm $(NASMFLAGS) $< -o $@

.PHONE: clean
clean:
	rm -rf $(BUILD) $(TARGET_ISO)

## Emulation stuff

.PHONY: bochs
bochs:
	bochs -q "com1: enabled=1, mode=file, dev=$(shell tty)"

.PHONY: run
run: $(TARGET_ISO)
	qemu-system-x86_64 -M q35 -m 2G -cdrom $< -boot d -serial stdio -display $(QEMU_DISPLAY_TYPE) \
-d mmu,int -D qemulog.txt -no-reboot -no-shutdown

.PHONY: debug
debug: $(TARGET_ISO)
	qemu-system-x86_64 -s -S -M q35 -m 2G -cdrom $< -boot d -serial stdio -display $(QEMU_DISPLAY_TYPE) \
-d mmu,int -D qemulog.txt -no-reboot -no-shutdown

.PHONY: gdb
gdb: $(BUILD)/iso/fors.elf
	gdb $< -ex "target remote :1234"


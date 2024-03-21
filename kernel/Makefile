KERNEL = fors.elf
BUILD  = build
SRC    = src

CC = gcc
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
    -I.                  	\
    -Isrc/include           \
    -Wno-unused-parameter	\
    -Wno-unused-function 	\
    -D__ARCH_$(ARCH)__   	\
    -ggdb                   \

LDFLAGS = \
	-nostdlib -static \
	-m elf_x86_64 \
	-z max-page-size=0x1000 \
	-T linker.ld \
	-no-pie

NASMFLAGS += \
    -f elf64 -g

CFILES = $(shell find src/generic -type f -name '*.c') $(shell find src/arch/$(ARCH) -type f -name '*.c')
NASMFILES := $(shell find src/generic -type f -name '*.asm') $(shell find src/arch/$(ARCH) -type f -name '*.asm')
OBJS = $(addprefix $(BUILD)/, $(CFILES:.c=.o) $(NASMFILES:.asm=.o))

.PHONY: all
all: $(KERNEL)

limine.h:
	curl https://raw.githubusercontent.com/limine-bootloader/limine/trunk/limine.h -o $@

$(KERNEL): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

$(BUILD)/src/%.o: $(SRC)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/src/%.o: $(SRC)/%.asm
	mkdir -p $(dir $@)
	nasm $(NASMFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(KERNEL) $(BUILD)

IMG_PREF = fors
ELF_EXE  = kernel/fors.elf
ARCH	 = x64
BUILD	 = build
export

.PHONY: all
all: $(IMG_PREF).iso

.PHONY: run
run: $(IMG_PREF).iso
	qemu-system-x86_64 -M q35 -m 2G -cdrom $< -boot d -serial stdio -display gtk

.PHONY: debug
debug: $(IMG_PREF).iso
	qemu-system-x86_64 -s -S -M q35 -m 2G -cdrom $< -boot d -serial stdio -display gtk

.PHONY: gdb
gdb: $(BUILD)/iso/fors.elf
	gdb $< -ex "target remote :1234"

.PHONY: kernel
kernel:
	make -C kernel

$(IMG_PREF).iso: kernel
	rm -rf $(BUILD)/iso

	mkdir -p $(BUILD)/iso

	cp -v $(ELF_EXE) \
		limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin \
		limine/limine-uefi-cd.bin \
		$(BUILD)/iso

	mkdir -p $(BUILD)/iso/EFI/BOOT

	cp -v limine/BOOTX64.EFI limine/BOOTIA32.EFI $(BUILD)/iso/EFI/BOOT/

	xorriso -as mkisofs -b limine-bios-cd.bin -no-emul-boot \
		-boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin -efi-boot-part \
		--efi-boot-image --protective-msdos-label \
		$(BUILD)/iso -o $@
		
	./limine/limine bios-install $@

.PHONY: clean
clean:
	rm -rf $(BUILD) $(IMG_PREF).iso
	make -C kernel clean
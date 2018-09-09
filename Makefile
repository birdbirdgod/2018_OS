all: BootLoader Disk.img

BootLoader:
	@echo
	@echo === Build Boot Loader ===
	@echo

	make -C 00.BootLoader

	@echo
	@echo === Build Complete ===
	@echo

Disk.img: 00.BootLoader/BootLoader.bin
	@echo
	@echo === Disk Image Build Start ===
	@echo

	cp 00.BootLoader/BootLoader.bin Disk.img

	@echo
	@echo === All Build Complete ===
	@echo

run:
	qemu-system-x86_64.exe -L . -m 64 -fda Disk.img -localtime -M pc

clean:
	make -C 00.BootLoader clean
	rm -f Disk.img

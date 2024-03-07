# change SIZE to reflect output of dasm build
SIZE = 657

all: firmware_ntsc_rom firmware_pal_rom firmware_pal60_rom firmware_uno_ntsc_rom firmware_uno_pal_rom firmware_uno_pal60_rom

firmware_ntsc_rom: *.asm
	dasm PlusCart.asm -DNTSC=1 -DPAL60=0 -DPAL50=0 -DSPINNER=0 -lPlusCart.lst -f3 -obin/$@.bin
	xxd -g1 -i -len $(SIZE) bin/$@.bin | head -n-1 | sed 's/bin_$@_bin/$@/g' > include/$@.h

firmware_pal_rom: *.asm
	dasm PlusCart.asm -DNTSC=0 -DPAL60=0 -DPAL50=1 -DSPINNER=0 -lPlusCart.lst -f3 -obin/$@.bin
	xxd -g1 -i -len 657 bin/$@.bin | head -n-1 | sed 's/bin_$@_bin/$@/g' > include/$@.h

firmware_pal60_rom: *.asm
	dasm PlusCart.asm -DNTSC=0 -DPAL60=1 -DPAL50=0 -DSPINNER=0 -lPlusCart.lst -f3 -obin/$@.bin
	xxd -g1 -i -len 657 bin/$@.bin | head -n-1 | sed 's/bin_$@_bin/$@/g' > include/$@.h

firmware_uno_ntsc_rom: *.asm
	dasm PlusCart.asm -DNTSC=1 -DPAL60=0 -DPAL50=0 -DSPINNER=1 -lPlusCart.lst -f3 -obin/$@.bin
	xxd -g1 -i -len 657 bin/$@.bin | head -n-1 | sed 's/bin_$@_bin/firmware_ntsc_rom/g' > include/$@.h

firmware_uno_pal_rom: *.asm
	dasm PlusCart.asm -DNTSC=0 -DPAL60=0 -DPAL50=1 -DSPINNER=1 -lPlusCart.lst -f3 -obin/$@.bin
	xxd -g1 -i -len 657 bin/$@.bin | head -n-1 | sed 's/bin_$@_bin/firmware_pal_rom/g' > include/$@.h

firmware_uno_pal60_rom: *.asm
	dasm PlusCart.asm -DNTSC=0 -DPAL60=1 -DPAL50=0 -DSPINNER=1 -lPlusCart.lst -f3 -obin/$@.bin
	xxd -g1 -i -len 657 bin/$@.bin | head -n-1 | sed 's/bin_$@_bin/firmware_pal60_rom/g' > include/$@.h

clean:
	rm -f bin/* include/*

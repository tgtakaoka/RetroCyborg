ASM=asm

.PHONY: clean

all: niblpatch.hex nibl1200bps.hex fbaspatch.hex echo.hex echoir.hex echoitr.hex

%.hex: %.asm
	$(ASM) -C ins8060 -l $*.lst -H16 -o $@ $^

clean:
	-rm -f *.lst *.hex

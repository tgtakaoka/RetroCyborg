ASM=asm -u

.PHONY: clean

all: echo.hex echoir.hex echoitr.hex rompatch.hex bb110bps.hex bb4800bps.hex

%.hex:	%.asm
	$(ASM) -l $*.lst -o $@ $^

clean:
	-rm -f *.lst *.hex

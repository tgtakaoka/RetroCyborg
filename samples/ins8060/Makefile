ASM=asm

.PHONY: clean

all: niblpatch.hex nibl1200bps.hex fbaspatch.hex \
     echo.hex echoir.hex echoitr.hex \
     arith.hex mandelbrot.hex

%.hex: %.asm
	$(ASM) -C ins8060 -l $*.lst -o $@ $^

clean:
	-rm -f *.lst *.hex

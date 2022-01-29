ASM=asm -u

.PHONY: clean

all: niblpatch.hex nibl1200bps.hex fbaspatch.hex

%.hex: %.asm
	$(ASM) -C ins8060 -u -l $*.lst -H16 -o $@ $^

clean:
	-rm -f *.lst *.hex

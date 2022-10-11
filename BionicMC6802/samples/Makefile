ASM=asm

.PHONY: clean

all: echo.s19 echoir.s19 echoitr.s19 mspatch.s19

%.s19:	%.asm
	$(ASM) -l $*.lst -o $@ $^

clean:
	-rm -f *.lst *.s19

ASM=asm -u

.PHONY: clean

all: echo.s19 echoir.s19 echoitr.s19

%.s19:	%.asm
	$(ASM) -l $*.lst -S16 -o $@ $^

clean:
	-rm -f *.lst *.s19

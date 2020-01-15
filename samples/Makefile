AS=asl -gnuerrors -L +t 0x1E -q
OBJCOPY=p2hex -l 32 -F Moto +5 -r '$$-$$' >/dev/null

.PHONY: clean

all: echo.s19 echoir.s19 echoitr.s19

%.p:	%.asm
	$(AS) $^
%.s19:	%.p
	$(OBJCOPY) $^
	@mv -f $*.hex $@

clean:
	-rm -f *.p *.lst *.s19

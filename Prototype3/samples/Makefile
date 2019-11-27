.PHONY: clean

all: echo.s19 echoir.s19 echoitr.s19

echo.s19: echo.asm
	sbasm $^

echoir.s19: echoir.asm
	sbasm $^

echoitr.s19: echoitr.asm
	sbasm $^

clean:
	-rm -f *.lst *.s19

SHELL=/bin/sh

xtract: xtract.c

debug: LDFLAGS += -g -O0
debug: xtract
.PHONY: debug

clean:
	-rm -f xtract

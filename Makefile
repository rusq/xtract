SHELL=/bin/sh

TILES_DIR=tiles

ifeq ($(DUKECONV),1)
CFLAGS += -DDUKECONV
endif

xtract: xtract.c

debug: LDFLAGS += -g -O0
debug: xtract
.PHONY: debug

clean:
	-rm -f xtract
	-rm -f *.o
	-rm -rf *.dSYM
.PHONY: clean 
clean_data:
	-rm -f *.ART *.MAP *.DAT
	-rm -rf maps tiles
.PHONY: clean_data

clean_all: clean clean_data
.PHONY: clean_all


format: xtract.c
	clang-format -i xtract.c
.PHONY: format

convert_tiles: $(TILES_DIR)
	cd $(CURDIR)/$(TILES_DIR) && mogrify -format png -transparent "#fc00fc" *.tga && rm *.tga	
	
.PHONY: conver_tiles

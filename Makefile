all: sivgb
.PHONY: sivgb

UNAME := $(shell uname)

OPTIMIZATION?=-O2
CFLAGS += -Wall -g $(OPTIMIZATION)

OBJ = sivgb.o \
z80.o \
mmu.o \
lcd.o

sivgb: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -rf sivgb *.o *~ sivgb.dSYM

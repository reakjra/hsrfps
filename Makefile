CC       = x86_64-w64-mingw32-gcc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11
LDFLAGS ?= -mwindows -s
LDLIBS  ?= -ladvapi32 -luser32

TARGET := hsrfps.exe

$(TARGET): hsrfps.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean

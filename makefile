# cc
CC := gcc
CFLAGS := -std=c99 \
	-O3 -Wall -Wextra -Wundef -Wfloat-equal -Wpointer-arith \
	-Wcast-align -Wstrict-prototypes -Wno-missing-field-initializers \
	-Warray-bounds -Wwrite-strings -pedantic -fstrict-aliasing \
	-Wshadow -Wswitch-default -Wswitch-enum

# dep
C_SRCS := $(shell find . -name "*.c")

# build
BUILD := build
OBJS :=
OBJS += $(patsubst %.c,$(BUILD)/%.o,$(C_SRCS))
CFLAGS += $(shell pkg-config --cflags json-c)
CFLAGS += $(shell pkg-config --cflags libcurl)
LDFLAGS :=
LDFLAGS += $(shell pkg-config --libs json-c)
LDFLAGS += $(shell pkg-config --libs libcurl)
LDLIBS :=
LDLIBS := -lpthread
BIN := cryptowatch

all: $(BIN)

pre: clean
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

%.c: %.h

$(BIN): pre $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS) $(LDFLAGS)

clean:
	rm -f cryptowatch
	rm -rf $(BUILD)

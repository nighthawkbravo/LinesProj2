ARCH ?= aarch64
CROSS_COMPILE ?= $(ARCH)-none-linux-gnu-

.PHONY: clean all

CC=$(CROSS_COMPILE)gcc
all: puzzle
LIBS := gpiod
OBJS := main.o
CFLAGS := $(CFLAGS) -ggdb

$(OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

puzzle: $(OBJS)
	$(CC) -o puzzle $(CFLAGS) $(LDFLAGS) $(OBJS) -l $(LIBS)

clean:
	rm -f $(OBJS) puzzle

ARCH ?= aarch64
CROSS_COMPILE ?= $(ARCH)-none-linux-gnu-

.PHONY: clean

CC=$(CROSS_COMPILE)gcc
OBJS := puzzle.o
puzzle: $(OBJS)
	$(CC) -o puzzle $(CFLAGS) $(LDFLAGS) $(OBJS) -l ncurses
$(OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS) puzzle

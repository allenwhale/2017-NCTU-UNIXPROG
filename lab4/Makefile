CC = g++
LDFLAGS = -lncurses
CFLAGS = -std=c++11 -O2
OUT = ./othello
OBJS = ./main.o \
	   ./board.o \
	   ./screen.o
DEPS := $(OBJS:%.o=%.o.d)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -MMD -MF $@.d -c $<

$(OUT): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(OUT) $(DEPS)

-include $(DEPS)


CC = g++
CFLAGS = -std=c++11 -O2
OUT = ./mysh
OBJS = ./command.o \
	   ./utils.o \
	   ./job.o \
	   ./main.o
DEPS := $(OBJS:%.o=%.o.d)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -MMD -MF $@.d -c $<

$(OUT): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	$(RM) $(OBJS) $(OUT) $(DEPS)

-include $(DEPS)

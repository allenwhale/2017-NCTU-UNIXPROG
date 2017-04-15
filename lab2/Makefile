CC=gcc
CFLAGS=-Wall -Wextra -Os -fno-stack-protector

OBJS=./utils.o \
	 ./inject.o

OUTPUT=./hw2.so

TEST=./test.out

all: $(OUTPUT)

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $*.o

$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) -fPIC -shared -o $(OUTPUT) $(OBJS) -ldl
	strip --strip-all $(OUTPUT)
	objcopy -R .note.gnu.build-id -R .comment $(OUTPUT) $(OUTPUT)

$(TEST): test.c $(OUTPUT)
	$(CC) $(CFLAGS) test.c -o $(TEST)

test: $(TEST) force
	rm -f /tmp/monitor.out
	MONITOR_OUTPUT=/tmp/monitor.out LD_PRELOAD=$(OUTPUT) $(TEST)
	python3 check.py /tmp/monitor.out

force:

clean:
	rm -f $(TEST) $(OUTPUT) $(OBJS)

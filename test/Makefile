CFLAGS=-Wall -Werror -g -O0 -ansi -pedantic -std=c99 -D_POSIX_C_SOURCE=199309L -I../include
LDFLAGS=-g -Wall -DDEV -ltfv -lstdc++ `pkg-config --libs opencv` -L../bin

OBJS=tinkervision_test.o
TEST=tinkervision_test.out

test: $(TEST)

all: test

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TEST): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	mv $@ ../bin/$(TEST)

clean:
	rm -f $(OBJS)
	rm -f $(TEST)

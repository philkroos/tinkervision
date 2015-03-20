CFLAGS=-Wall -Werror -g -O0 -ansi -pedantic -std=c99 -D_POSIX_C_SOURCE=199309L -I../include/api
LDFLAGS=-g -Wall -DDEV -L../build -ltfv -lstdc++ `pkg-config --libs opencv`

OBJ=tinkervision_test.o
TEST=tfv-test.out

all: test

test: $(TEST)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TEST): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

clean:
	rm -f $(OBJ)
	rm -f $(TEST)

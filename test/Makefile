CC=g++
CFLAGS=-Wall -Werror -g -O0 -ansi -pedantic -std=c++11 -D_POSIX_C_SOURCE=199309L

INC=-I../include/api -I../include/camera `pkg-config --cflags opencv`
LDFLAGS=-g -Wall -DDEV -L../build -ltfv -lstdc++ `pkg-config --libs opencv` -lv4l2
CONVLDFLAGS=-g -Wall -L../build -ltfv -lstdc++ `pkg-config --libs opencv`

TESTOBJ=tinkervision_test.o
TEST=tfv-test.out
CONVOBJ=tfv_conversions.o
CONV=tfv-conversions.out

all: test conv

test: $(TEST)
conv: $(CONV)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

%.o: %.cc
	$(CC) $(CFLAGS) $(INC) -c $<

$(TEST): $(TESTOBJ)
	$(CC) $(CFLAGS) $(INC) $(TESTOBJ) -o $@ $(LDFLAGS)

$(CONV): $(CONVOBJ)
	$(CC) $(CFLAGS) $(INC) $(CONVOBJ) -o $@ $(CONVLDFLAGS)

clean:
	rm -f $(CONVOBJ) $(TESTOBJ) $(TEST) $(CONV)

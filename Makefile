CFLAGS=-Wall -Werror -g -O0 -ansi -pedantic -std=c99
CCFLAGS=-Wall -Werror -g -O0 -std=c++11 -DDEV `pkg-config --cflags opencv`
ARFLAGS=-rcs
LDFLAGS_TEST=-g -Wall -lstdc++  -ltfv `pkg-config --libs opencv` -L`pwd`
LDFLAGS_LIB=-g -Wall -lstdc++  `pkg-config --libs opencv`

TEST_OBJS=tinkervision_test.o tinkervision.o
TEST=tinkervision_test.out
LIB_OBJS=tinkervision.o api.o feature.o cameracontrol.o \
	camera.o colortracker.o window.o
LIB=tinkervision

lib: $(LIB)
test: $(TEST)

all: lib test

%.o: %.cc
	$(CC) $(CCFLAGS) -c $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TEST): $(TEST_OBJS)
	$(CC) $(TEST_OBJS) $(LDFLAGS_TEST) -o $@

$(LIB): $(LIB_OBJS)
	$(AR) $(ARFLAGS) libtfv.a $(LIB_OBJS)

clean:
	rm -f $(LIB_OBJS) $(TEST_OBJS)
	rm -f $(TEST) $(LIB)

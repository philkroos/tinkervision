CCFLAGS=-Wall -Werror -g -O0 -std=c++11 -DDEV `pkg-config --cflags opencv` -I../include
ARFLAGS=-rcs
LDFLAGS_LIB=-g -Wall -lstdc++  `pkg-config --libs opencv`

OBJS=tinkervision.o api.o feature.o cameracontrol.o \
	camera.o colortracker.o window.o
LIB=tinkervision

lib: $(LIB)

all: lib

%.o: %.cc
	$(CC) $(CCFLAGS) -c $<

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) libtfv.a $(OBJS)
	rm -rf ../bin && mkdir ../bin && mv libtfv.a ../bin

clean:
	rm -f $(OBJS)
	rm -f $(LIB)

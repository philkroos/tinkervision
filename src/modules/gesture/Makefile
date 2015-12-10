CC		:= g++
CCFLAGS	:= -Wall -Werror -std=c++14 -shared -fPIC -DWITH_LOGGER

INC		:= `pkg-config --cflags opencv`
LDFLAGS	:= `pkg-config --libs opencv`

ifeq ($(DEBUG),yes)
	CCFLAGS	+= -g -O0 -DDEBUG
else
	CCFLAGS	+= -O3
endif

ifndef PRE
	PRE = /usr/lib/tinkervision
endif

LDFLAGS	+= -L/usr/lib/python2.7 -lpython2.7
INC		+= -I/usr/include/python2.7 -I/usr/include/tinkervision

SO		:= gesture.so
TV		:= interface/module.o interface/image.o interface/parameter.o
OBJ		:= gesture.o detect.o hand.o pixel.o $(TV)

all: so

%.o: %.cc
	$(CC) $(CCFLAGS) -c $< -o $@ $(INC)

so: $(OBJ)
	$(CC) -shared -o $(SO) $(OBJ) $(LDFLAGS)

.phony:
	clean

clean:
	rm -f *.o *.so interface/*.o

install: $(SO)
	install -m 544 $(SO) $(PRE)/$(SO)

.PHONY: install

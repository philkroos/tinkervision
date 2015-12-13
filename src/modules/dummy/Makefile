CC		:= g++
CCFLAGS	:= -Wall -Werror -std=c++14 -shared -fPIC

ifeq ($(DEBUG),yes)
	CCFLAGS	+= -g -O0
else
	CCFLAGS	+= -O3
endif

ifndef PRE
	PRE = /usr/lib/tinkervision
endif

INC		:= -I/usr/include/tinkervision

SO		:= dummy.so
TV		:= interface/module.o interface/image.o \
		   interface/parameter.o interface/environment.o
OBJ		:= dummy.o $(TV)

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

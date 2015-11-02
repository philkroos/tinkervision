CC		:= g++

INC	:= -I/usr/include/tinkervision
ifeq ($(DEBUG),yes)
	CCFLAGS	:= -Wall -Werror -g -O0 -std=c++11 -shared -fPIC -DDEBUG
	INC		+= `pkg-config --cflags opencv`
	LDFLAGS	:= `pkg-config --libs opencv`
else
	CCFLAGS	:= -Wall -Werror -O3 -std=c++11 -shared -fPIC
endif

SO		:= grayfilter.so
OBJ		:= module.o grayfilter.o

all: so

module.o:
	$(CC) $(CCFLAGS) -c ../../lib/interface/module.cc -o $@ $(INC)

%.o: %.cc
	$(CC) $(CCFLAGS) -c $< -o $@ $(INC)

so: $(OBJ)
	$(CC) -shared -o $(SO) $(OBJ) $(LDFLAGS)

.phony:
	clean

clean:
	rm -f *.o *.so

# installation to additional path
prefix	:= /tmp

install: $(SO)
	mkdir -p $(prefix)/lib/tinkervision/
	install -m 544 $(SO) $(prefix)/lib/tinkervision/$(SO)

.PHONY: install

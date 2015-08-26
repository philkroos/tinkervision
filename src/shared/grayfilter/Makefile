CC		:= g++
CCFLAGS		:= -Wall -Werror -g -O0 -std=c++11 -shared -fPIC -DDEBUG

INC		:= -I/usr/local/include `pkg-config --cflags opencv`
LDFLAGS	:= `pkg-config --libs opencv`

SO		:= grayfilter.so
OBJ		:= grayfilter.o

all: so

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

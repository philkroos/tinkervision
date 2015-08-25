CC		:= g++
CCFLAGS		:= -Wall -Werror -O3 -std=c++11 -shared -fPIC

INC		:= -I/usr/local/include `pkg-config --cflags opencv`
LDFLAGS		:= `pkg-config --libs opencv`

SO		:= stream.so
OBJ		:= stream.o

all: so

%.o: %.cc
	$(CC) $(CCFLAGS) -c $< -o $@ $(INC)

so: $(OBJ)
	$(CC) -shared -o $(SO) $(OBJ) $(LDFLAGS)

.phony:
	clean

clean:
	rm -f *.o *.so

# installation to system path
prefix	:= /usr/local

install: $(SO)
	mkdir -p $(prefix)/lib/tinkervision/
	install -m 544 $(SO) $(prefix)/lib/tinkervision/$(SO)

.PHONY: install

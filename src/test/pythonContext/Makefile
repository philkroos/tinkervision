CC	:= g++
CCFLAGS := -Wall -Werror -g -O0 -ansi -pedantic -std=c++11 -DWITH_LOGGER

INC	:= -I../ -I../../lib/core \
	   -I../../lib/tools \
	   -I../../lib/imaging \
	   -I/usr/include/python2.7
LDFLAGS := -g -Wall -lstdc++ -L/usr/lib/python2.7 -lpython2.7

TV_OBJ	:= ../../../build/core/environment.o \
	   ../../../build/tools/filesystem.o \
	   ../../../build/core/logger.o \
	   ../../../build/core/python_context.o
OBJ	:= tfv_python_context.o
OUT	:= tfv-python-context

all: py-ctxt

py-ctxt: $(OUT)

%.o: %.c
	$(CC) $(CCFLAGS) $(INC) -c $<

%.o: %.cc
	$(CC) $(CCFLAGS) $(INC) -c $<

$(OUT): $(OBJ)
	$(CC) $(CCFLAGS) $(INC) $(OBJ) $(TV_OBJ) -o $@ $(LDFLAGS)

clean:
	@rm -f $(OBJ) $(OUT)

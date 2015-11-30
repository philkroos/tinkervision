CC	:= g++
usr_prefix = $(HOME)/tv
CCFLAGS := -Wall -Werror -g -std=c++11 -O0 -pedantic -DDEBUG -DWITH_LOGGER -DUSR_PREFIX=\"$(usr_prefix)\"

INC	:= -I../../lib/core \
	   -I../../lib/imaging \
	   -I../../lib/interface \
	   -I../../lib/tools \
	   -I/usr/include/python2.7

LDFLAGS := -g -Wall -lstdc++ -ltinkervision -ldl -pthread -L/usr/lib/python2.7 -lpython2.7

TFV_UNITS := ../../../build/interface/module.o \
	     ../../../build/interface/parameter.o \
	     ../../../build/interface/image.o \
	     ../../../build/core/module_loader.o \
	     ../../../build/core/module_wrapper.o \
	     ../../../build/core/logger.o \
	     ../../../build/tools/filesystem.o \
	     ../../../build/tools/dirwatch.o \
	     ../../../build/interface/environment.o \
	     ../../../build/core/python_context.o
OBJ	:= tfv_test_loader.o
OUT	:= tfv-test-loader

all: test

test: $(OUT)

%.o: %.c
	$(CC) $(CCFLAGS) $(INC) -c $<

%.o: %.cc
	$(CC) $(CCFLAGS) $(INC) -c $<

$(OUT): $(OBJ)
	$(CC) $(CCFLAGS) $(INC) $(TFV_UNITS) $(OBJ) -o $(OUT) $(LDFLAGS)

clean:
	@rm -f $(OBJ) $(OUT)

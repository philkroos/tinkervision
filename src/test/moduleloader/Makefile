CC	:= g++
usr_prefix = $(HOME)/tv
CCFLAGS := -Wall -Werror -g -std=c++11 -O0 -pedantic -DDEBUG -DUSR_PREFIX=\"$(usr_prefix)\"

INC	:= -I../../lib/core \
	   -I../../lib/imaging \
	   -I../../lib/interface \
	   -I../../lib/tools \
	   -I/usr/include/python2.7

LDFLAGS := -g -Wall -lstdc++ -ldl -pthread -L/usr/lib/python2.7 -lpython2.7

TFV_UNITS := ../../lib/interface/module.cc \
	     ../../lib/interface/parameter.cc \
	     ../../lib/core/module_loader.cc \
	     ../../lib/core/module_wrapper.cc \
	     ../../lib/core/logger.cc \
	     ../../lib/imaging/image.cc \
	     ../../lib/tools/filesystem.cc \
	     ../../lib/tools/dirwatch.cc \
	     ../../lib/core/environment.cc \
	     ../../lib/core/python_context.cc
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

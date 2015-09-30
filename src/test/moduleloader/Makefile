CC	:= g++
CCFLAGS := -Wall -Werror -g -std=c++11 -O0 -pedantic

INC	:= -I../../lib/core -I../../lib/imaging -I../../lib/interface
LDFLAGS := -g -Wall -lstdc++ -ldl

TFV_UNITS := ../../lib/core/module_loader.cc \
             ../../lib/core/module.cc \
	     ../../lib/core/logger.cc \
	     ../../lib/core/filesystem.cc
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

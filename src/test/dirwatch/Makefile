CC	:= g++
CCFLAGS := -Wall -Werror -g -std=c++11 -O0 -pedantic

INC	:= -I../../lib/core -I../../lib/tools
LDFLAGS := -g -Wall -lstdc++ -lpthread

TV_OBJ	:= ../../lib/tools/filesystem.cc ../../lib/tools/dirwatch.cc
OBJ	:= tfv_test_dirwatch.o
OUT	:= tfv-test-dirwatch

all: test

test: $(OUT)

%.o: %.c
	$(CC) $(CCFLAGS) $(INC) -c $<

%.o: %.cc
	$(CC) $(CCFLAGS) $(INC) -c $<

$(OUT): $(OBJ)
	$(CC) $(CCFLAGS) $(INC) $(TV_OBJ) $(OBJ) -o $(OUT) $(LDFLAGS)

clean:
	@rm -f $(OBJ) $(OUT)

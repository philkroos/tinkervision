CC	:= g++
CCFLAGS := -Wall -Werror -g -O0 -ansi -pedantic -std=c++11 -D_POSIX_C_SOURCE=199309L

INC	:= -I../../src/api -I../../src/imaging `pkg-config --cflags opencv`
LDFLAGS := -g -Wall -L../../build -ltfv -lstdc++ -lv4l2 `pkg-config --libs opencv`

OBJ	:= tfv_colortrack.o
OUT	:= tfv-colortrack

all: test

test: $(OUT)

%.o: %.c
	$(CC) $(CCFLAGS) $(INC) -c $<

%.o: %.cc
	$(CC) $(CCFLAGS) $(INC) -c $<

$(OUT): $(OBJ)
	$(CC) $(CCFLAGS) $(INC) $(OBJ) -o $(OUT) $(LDFLAGS)

clean:
	@rm -f $(OBJ) $(OUT)

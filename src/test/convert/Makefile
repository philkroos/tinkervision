CC	:= g++
CCFLAGS := -Wall -Werror -g -O0 -ansi -pedantic -std=c++11 -D_POSIX_C_SOURCE=199309L

INC	:= -I../ -I../../lib/imaging -I/usr/include/tinkervision \
		`pkg-config --cflags opencv`
LDFLAGS := -g -Wall -ltinkervision -lstdc++ `pkg-config --libs opencv`

OBJ	:= tfv_convert.o
OUT	:= tfv-convert

all: conv

conv: $(OUT)

%.o: %.c
	$(CC) $(CCFLAGS) $(INC) -c $<

%.o: %.cc
	$(CC) $(CCFLAGS) $(INC) -c $<

$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

clean:
	@rm -f $(OBJ) $(OUT)

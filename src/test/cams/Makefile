CC	:= g++
CCFLAGS := -Wall -Werror -g -O0 -std=c++11 -DDEBUG -DWITH_LOGGER

INC	:= `pkg-config --cflags opencv`
LDFLAGS := -g -Wall -lstdc++ -ltinkervision -lm -lv4l2

OBJ	:= tv_cams.o
OUT	:= tv-cams

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

CC	:= gcc
CCFLAGS := -Wall -Werror -g -O0 -ansi -pedantic -DDEBUG -D_POSIX_C_SOURCE=199309L

INC	:= -I../../src/api -I../../src/imaging `pkg-config --cflags opencv`
LDFLAGS := -g -Wall -L../../build -ltfv -lstdc++ -lv4l2 `pkg-config --libs opencv`

OBJ1	:= tfv_scenes.o
OUT1	:= tfv-scenes

all: test1

test1: $(OUT1)

%.o: %.c
	$(CC) $(CCFLAGS) $(INC) -c $<

%.o: %.cc
	$(CC) $(CCFLAGS) $(INC) -c $<

$(OUT1): $(OBJ1)
	$(CC) $(CCFLAGS) $(INC) $(OBJ1) -o $(OUT1) $(LDFLAGS)

clean:
	@rm -f $(OBJ1) $(OUT1)

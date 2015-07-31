CC	:= gcc
CCFLAGS := -Wall -Werror -g -O0 -ansi -pedantic -DDEBUG -D_POSIX_C_SOURCE=199309L

INC	:= -I../../src/api -I../../src/imaging `pkg-config --cflags opencv`
LDFLAGS := -g -Wall -L../../build -ltfv -lstdc++ -lv4l2 `pkg-config --libs opencv`

OBJ1	:= tfv_scenes.o
OUT1	:= tfv-scenes
OBJ2	:= tfv_scenetrees.o
OUT2	:= tfv-scenetrees

all: test1 test2

test1: $(OUT1)
test2: $(OUT2)

%.o: %.c
	$(CC) $(CCFLAGS) $(INC) -c $<

%.o: %.cc
	$(CC) $(CCFLAGS) $(INC) -c $<

$(OUT1): $(OBJ1)
	$(CC) $(CCFLAGS) $(INC) $(OBJ1) -o $(OUT1) $(LDFLAGS)

$(OUT2): $(OBJ2)
	$(CC) $(CCFLAGS) $(INC) $(OBJ2) -o $(OUT2) $(LDFLAGS)

clean:
	@rm -f $(OBJ1) $(OUT1) $(OBJ2) $(OUT2)

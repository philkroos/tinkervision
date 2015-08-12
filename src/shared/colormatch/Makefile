CC		:= g++
CCFLAGS	:= -Wall -Werror -g -O0 -std=c++11 -shared -fPIC -DDEBUG

FOLDER		:= api imaging debug
INC		:= $(addprefix -I/home/philipp/tinkervision/src/,$(FOLDER))

SO		:= colormatch.so
OBJ		:= colormatch.o

all: so

%.o: %.cc
	$(CC) $(CCFLAGS) -c $< -o $@ $(INC)

so: $(OBJ)
	$(CC) -shared -o $(SO) $(OBJ) $(LDFLAGS)

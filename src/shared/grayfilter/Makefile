CC		:= g++
CCFLAGS	:= -Wall -Werror -g -O0 -std=c++11 -shared -fPIC -DDEBUG

FOLDER		:= interface
INC		:= $(addprefix -I/home/philipp/tinkervision/src/,$(FOLDER))

SO		:= grayfilter.so
OBJ		:= grayfilter.o

all: so

%.o: %.cc
	$(CC) $(CCFLAGS) -c $< -o $@ $(INC)

so: $(OBJ)
	$(CC) -shared -o $(SO) $(OBJ) $(LDFLAGS)

.phony:
	clean

clean:
	rm -f *.o *.so

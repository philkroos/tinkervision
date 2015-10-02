CC		:= gcc
CFLAGS		:= -Wall -Werror -g -O0 -ansi -pedantic -D_POSIX_C_SOURCE=199309L
LIBS		:= -ltinkervision -lstdc++ -lv4l2

OBJ		:= tfv_snapshot.o
OUT		:= tfv-snapshot

all: snapshot

snapshot: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(OUT) $(LIBS)

clean:
	@rm -f $(OBJ) $(OUT)

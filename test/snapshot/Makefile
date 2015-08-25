CC		:= gcc
CFLAGS		:= -Wall -Werror -g -O0 -ansi -pedantic -D_POSIX_C_SOURCE=199309L
INC		:= `pkg-config --cflags opencv`
LIBS_OPENCV	:= `pkg-config --libs opencv`
LIBS		:= -ltinkervision -lstdc++ -lv4l2 \
	           $(LIBS_OPENCV) $(LIBS_EXTERNAL)  $(LIB_X264) $(LIBS_LIVE)

OBJ		:= tfv_snapshot.o
OUT		:= tfv-snapshot

all: snapshot

snapshot: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(INC) $(OBJ) -o $(OUT) $(LIBS)

clean:
	@rm -f $(OBJ) $(OUT)

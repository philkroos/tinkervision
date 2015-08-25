CC		:= gcc
CFLAGS		:= -Wall -Werror -g -O0 -ansi -pedantic -D_POSIX_C_SOURCE=199309L
INC		:= `pkg-config --cflags opencv`
LIBS_X264	:= -lx264
LIBS_LIVE	:= -lBasicUsageEnvironment -lUsageEnvironment -lgroupsock -lliveMedia
LIBS_OPENCV	:= `pkg-config --libs opencv`
LIBS		:= -ltinkervision -lstdc++ -lv4l2 \
	           $(LIBS_OPENCV) $(LIBS_EXTERNAL)  $(LIB_X264) $(LIBS_LIVE)

OBJ		:= tfv_record.o
OUT		:= tfv-record

all: record

record: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(INC) $(OBJ) -o $(OUT) $(LIBS)

clean:
	@rm -f $(OBJ) $(OUT)

CC		:= g++
CCFLAGS	:= -Wall -Werror -O3 -std=c++11 -shared -fPIC -DWITH_LOGGER

# Includes for LIVE555 streamer and opencv
LIVE_MODULES	:= BasicUsageEnvironment UsageEnvironment groupsock liveMedia
INC		:= $(addprefix -I/usr/local/include/,$(LIVE_MODULES))

# Libraries for live555 streamer, video compression and opencv
LIBS_LIVE       := $(addprefix -l,$(LIVE_MODULES))
LIBS_X264	:= -lx264
LDFLAGS	:= $(LIBS_X264) $(LIBS_LIVE) -rdynamic

ifndef PRE
	PRE = /usr/lib/tinkervision
endif

ifeq ($(DEBUG),yes)
	CCFLAGS	:= -Wall -Werror -g -O0 -std=c++11 -shared -fPIC -DDEBUG
	INC		+= `pkg-config --cflags opencv`
	LDFLAGS	+= `pkg-config --libs opencv`
endif

LDFLAGS	+= -L/usr/lib/python2.7 -lpython2.7
INC		+= -I/usr/include/python2.7 -I/usr/include/tinkervision

SO		:= stream.so
TV		:= interface/module.o interface/image.o interface/parameter.o
OBJ		:= stream.o $(TV) \
		   h264_media_session.o execution_context.o \
		   h264_encoder.o h264_byte_source.o

all: so

%.o: %.cc
	$(CC) $(CCFLAGS) -c $< -o $@ $(INC)

so: $(OBJ)
	$(CC) -shared -o $(SO) $(OBJ) $(LDFLAGS)

.phony:
	clean

clean:
	rm -f *.o *.so interface/*.o

# installation to system path
prefix	:= /usr/

install: $(SO)
	install -m 544 $(SO) $(PRE)/$(SO)

.PHONY: install

CC		:= g++
CCFLAGS	:= -Wall -Werror -g -O0 -std=c++11 -D_GLIBCXX_USE_NANOSLEEP -fPIC -DDEBUG

# structure
MODULES         := colormatch stream record motiondetect
PARTS		:= api imaging debug modules $(addprefix modules/,$(MODULES))
BUILD_PREFIX	:= build
BUILD_DIR	:= $(addprefix $(BUILD_PREFIX)/,$(PARTS))
SRC_PREFIX	:= src
SRC_DIR	:= $(addprefix $(SRC_PREFIX)/,$(PARTS))
TEST_DIR	:= test

# LIVE555 streamer
LIVE_MODULES	:= BasicUsageEnvironment UsageEnvironment groupsock liveMedia

# Libraries
LIBS_LIVE       := $(addprefix -l,$(LIVE_MODULES))
LIBS_X264	:= -lx264
LIBS_OPENCV	:= `pkg-config --libs opencv`
LIBS_SYSTEM	:= -lstdc++ -lv4l2 -lm
LDFLAGS	:= $(LIBS_SYSTEM) $(LIBS_OPENCV) $(LIBS_X264) $(LIBS_LIVE)

# Header
OCV_INC	:= `pkg-config --cflags opencv`
LIVE_INC	:= $(addprefix -I/usr/include/,$(LIVE_MODULES))
INC             := $(addprefix -I./src/,$(PARTS)) $(OCV_INC) $(LIVE_INC)

# files
SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cc))
OBJ		:= $(patsubst src/%.cc,build/%.o,$(SRC)) # $(OBJ_EXTERNAL)


# binary targets (shared library)
LIB		:= build/libtfv.so

all: directories lib #test
.PHONY: test clean

# search paths
vpath %.cc $(SRC_DIR)

# generates targets, called at bottom
define make-goal
$1/%.o: %.cc
	$(CC) $(CCFLAGS) -c $$< -o $$@ $(INC)
endef

# setup directory structure
$(BUILD_DIR):
	@mkdir -p $@

directories: $(BUILD_DIR)


# actual targets
lib: directories $(OBJ)
	$(CC) -shared -o $(LIB) $(OBJ) $(LDFLAGS)

test: $(LIB)
	cd $(TEST_DIR) && make

clean:
	@rm -rf $(BUILD_PREFIX)
	cd $(TEST_DIR) && make clean

# generate rules
$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))


# documentation
doc:
	doxygen

# installation
prefix	:= /usr/local
EXP_HEADER	:= src/api/tinkervision.h src/api/tinkervision_defines.h

install: $(LIB)
	install -m 544 $(LIB) $(prefix)/lib/libtinkervision.so
	install -m 544 $(EXP_HEADER) $(prefix)/include/

.PHONY: install

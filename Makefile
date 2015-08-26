CC		:= g++
CCFLAGS_DEBUG	:= -Wall -Werror -g -O0 -std=c++11 -fPIC -DDEBUG
CCFLAGS	:= -Wall -Werror -O3 -std=c++11 -fPIC

# structure
PARTS		:= core imaging debug interface
BUILD_PREFIX	:= build
BUILD_DIR	:= $(addprefix $(BUILD_PREFIX)/,$(PARTS))
SRC_PREFIX	:= src/lib
SRC_DIR	:= $(addprefix $(SRC_PREFIX)/,$(PARTS))
TEST_DIR	:= src/test

#LIBS_OPENCV	:= `pkg-config --libs opencv`
LIBS_OPENCV	:= /usr/local/lib/libopencv_highgui.so \
		   /usr/local/lib/libopencv_imgproc.so \
                   /usr/local/lib/libopencv_video.so \
                   -lrt -lpthread -lm -ldl
LIBS_SYSTEM	:= -lstdc++ -lv4l2 -lm
LDFLAGS	:= $(LIBS_SYSTEM) $(LIBS_OPENCV) -rdynamic

# Header
#OCV_INC	:= `pkg-config --cflags opencv`
OCV_INC	:= -I/usr/local/include/opencv -I/usr/local/include
INC             := $(addprefix -I./$(SRC_PREFIX)/,$(PARTS)) $(OCV_INC)

# files
SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cc))
OBJ		:= $(patsubst $(SRC_PREFIX)/%.cc,build/%.o,$(SRC))
OBJ_DBG	:= $(patsubst $(SRC_PREFIX)/%.cc,build/%_dbg.o,$(SRC))


# binary targets
LIB_REL	:= build/libtinkervision.so
LIB_DBG	:= build/libtinkervision_dbg.so

all: directories release debug
release: directories lib
debug: directories lib-debug

.PHONY: test clean

# search paths
vpath %.cc $(SRC_DIR)

# generates targets, called at below
define make-goal-debug
$1/%_dbg.o: %.cc
	$(CC) $(CCFLAGS_DEBUG) -c $$< -o $$@ $(INC)
endef
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
	$(CC) -shared -o $(LIB_REL) $(OBJ) $(LDFLAGS)

lib-debug: directories $(OBJ_DBG)
	$(CC) -shared -o $(LIB_DBG) $(OBJ_DBG) $(LDFLAGS)

test: $(LIB_DBG)
	cd $(TEST_DIR) && make

clean:
	@rm -rf $(BUILD_PREFIX)
	cd $(TEST_DIR) && make clean

# generate rules
$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal-debug,$(bdir))))
$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))


# documentation
doc:
	doxygen

# installation
prefix		:= /usr/local
EXP_HEADER	:= $(SRC_PREFIX)/core/tinkervision.h \
		   $(SRC_PREFIX)/core/tinkervision_defines.h \
                   $(SRC_PREFIX)/interface/tv_module.hh \
		   $(SRC_PREFIX)/core/exceptions.hh \
                   $(SRC_PREFIX)/imaging/image.hh
EXP_HEADER_DBG	:= $(EXP_HEADER) \
		   $(SRC_PREFIX)/debug/logger.hh

install: $(LIB_REL)
	install -m 544 $(LIB_REL) $(prefix)/lib/libtinkervision.so
	install -m 544 $(EXP_HEADER) $(prefix)/include/

install_dbg: $(LIB_DBG)
	install -m 544 $(LIB_DBG) $(prefix)/lib/libtinkervision_dbg.so
	install -m 544 $(EXP_HEADER_DBG) $(prefix)/include/

.PHONY: install

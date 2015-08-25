CC		:= g++
CCFLAGS_DEBUG	:= -Wall -Werror -g -O0 -std=c++11 -fPIC -DDEBUG
CCFLAGS		:= -Wall -Werror -O3 -std=c++11 -fPIC

# structure
MODULES         := stream record
PARTS		:= api imaging modules debug interface \
		   $(addprefix modules/,$(MODULES))
BUILD_PREFIX	:= build
BUILD_DIR	:= $(addprefix $(BUILD_PREFIX)/,$(PARTS))
SRC_PREFIX	:= src
SRC_DIR		:= $(addprefix $(SRC_PREFIX)/,$(PARTS))
TEST_DIR	:= test

# LIVE555 streamer
LIVE_MODULES	:= BasicUsageEnvironment UsageEnvironment groupsock liveMedia

# Libraries
LIBS_LIVE       := $(addprefix -l,$(LIVE_MODULES))
LIBS_X264	:= -lx264
LIBS_OPENCV	:= `pkg-config --libs opencv`
LIBS_SYSTEM	:= -lstdc++ -lv4l2 -lm
LDFLAGS		:= $(LIBS_SYSTEM) $(LIBS_OPENCV) $(LIBS_X264) $(LIBS_LIVE)

# Header
OCV_INC		:= `pkg-config --cflags opencv`
LIVE_INC	:= $(addprefix -I/usr/include/,$(LIVE_MODULES))
INC             := $(addprefix -I./src/,$(PARTS)) $(OCV_INC) $(LIVE_INC)

# files
SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cc))
OBJ		:= $(patsubst src/%.cc,build/%.o,$(SRC))
OBJ_DBG		:= $(patsubst src/%.cc,build/%_dbg.o,$(SRC))


# binary targets
LIB_REL		:= build/libtinkervision.so
LIB_DBG		:= build/libtinkervision_dbg.so

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
EXP_HEADER	:= src/api/tinkervision.h \
		   src/api/tinkervision_defines.h \
                   src/interface/tv_module.hh
EXP_HEADER_DBG	:= $(EXP_HEADER) \
                   src/imaging/image.hh \
		   src/debug/logger.hh

install: $(LIB_REL)
	install -m 544 $(LIB_REL) $(prefix)/lib/libtinkervision.so
	install -m 544 $(EXP_HEADER) $(prefix)/include/

install_dbg: $(LIB_DBG)
	install -m 544 $(LIB_DBG) $(prefix)/lib/libtinkervision_dbg.so
	install -m 544 $(EXP_HEADER_DBG) $(prefix)/include/

.PHONY: install

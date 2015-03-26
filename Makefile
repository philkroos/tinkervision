CCFLAGS	:=-Wall -Werror -g -O0 -std=gnu++11 -D_GLIBCXX_USE_NANOSLEEP
ARFLAGS	:=-rcs
LDFLAGS_LIB	:=-g -Wall -lstdc++  `pkg-config --libs opencv` -lv4l2

# externals
OCV_INC	:= `pkg-config --cflags opencv`

# structure
MODULES	:= colortrack
PARTS		:= api imaging modules $(addprefix modules/,$(MODULES))
BUILD_PREFIX	:= build
BUILD_DIR	:= $(addprefix $(BUILD_PREFIX)/,$(PARTS))
SRC_PREFIX	:= src
SRC_DIR	:= $(addprefix $(SRC_PREFIX)/,$(PARTS))
TEST_DIR	:= test

# files
SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cc))
OBJ		:= $(patsubst src/%.cc,build/%.o,$(SRC))
INC             := $(addprefix -I./src/,$(PARTS)) $(OCV_INC)

$(info $$MODULES is [${MODULES}])
$(info $$PARTS is [${PARTS}])
$(info $$SRC_PREFIX is [${SRC_PREFIX}])


# binary targets
LIB		:= build/libtfv.a

all: directories lib test
.PHONY: test clean

# search paths
vpath %.cc $(SRC_DIR)

# generates targets, called at bottom
define make-goal
$1/%.o: %.cc
	$(CC) $(CCFLAGS) $(INC) -c $$< -o $$@
endef

# setup directory structure
$(BUILD_DIR):
	@mkdir -p $@

directories: $(BUILD_DIR)


# actual targets
lib: $(OBJ)
	$(AR) $(ARFLAGS) $(LIB) $(OBJ)

test: $(LIB)
	cd $(TEST_DIR) && make

clean:
	@rm -rf $(BUILD_PREFIX)
	cd $(TEST_DIR) && make clean

# generate rules
$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))

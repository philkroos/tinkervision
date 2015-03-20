CCFLAGS	:=-Wall -Werror -g -O0 -std=gnu++11 -D_GLIBCXX_USE_NANOSLEEP
ARFLAGS	:=-rcs
LDFLAGS_LIB	:=-g -Wall -lstdc++  `pkg-config --libs opencv`

# externals
OCV_INC	:= `pkg-config --cflags opencv`

# structure
MODULES	:= api camera modules
BUILD_PREFIX	:= build
BUILD_DIR	:= $(addprefix $(BUILD_PREFIX)/,$(MODULES))
SRC_DIR	:= $(addprefix src/,$(MODULES))
TEST_DIR	:= test

# files
SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cc))
OBJ		:= $(patsubst src/%.cc,build/%.o,$(SRC))
INC             := $(addprefix -I./include/,$(MODULES)) $(OCV_INC)

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

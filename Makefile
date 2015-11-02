cc		:= g++
ccflags	:= -Wall -Werror -pedantic -shared -std=c++11 -fpic -DWITH_LOGGER

ifdef DEBUG
	ccflags += -g -O0 -DDEBUG
else
	ccflags += -O3
endif

# structure
parts		:= core tools imaging debug interface
build_prefix	:= build
build_dir	:= $(addprefix $(build_prefix)/,$(parts))
src_prefix	:= src/lib
src_dir	:= $(addprefix $(src_prefix)/,$(parts))

libs		:= -lstdc++ -lv4l2 -lm
inc		:= $(addprefix -I./$(src_prefix)/,$(parts)) $(OCV_inc)
ifneq ($(or $(WITH_OPENCV_CAM),$(DEBUG_WINDOW)),)
	libs	+= -lopencv_highgui -lopencv_imgproc -lopencv_video \
		   -lrt -lpthread -ldl
	inc	+= -I/usr/local/include/opencv -I/usr/local/include
endif

ldflags	:= $(libs) -rdynamic

# files
src		:= $(foreach sdir,$(src_dir),$(wildcard $(sdir)/*.cc))
obj		:= $(patsubst $(src_prefix)/%.cc,build/%.o,$(src))
output		:= libtinkervision.so


all: directories lib

.PHONY: clean

# search paths
vpath %.cc $(src_dir)

# generates targets, called at below
define make-goal
$1/%.o: %.cc
	$(cc) $(ccflags) -c $$< -o $$@ $(inc)
endef

# setup directory structure
$(build_dir):
	@mkdir -p $@

directories: $(build_dir)

# actual targets
lib: directories $(obj)
	$(cc) -shared -o $(build_prefix)/$(output) $(obj) $(ldflags)
ifndef DEBUG
	strip $(build_prefix)/$(output)
endif

clean:
	@rm -rf $(build_prefix)

# generate rules
$(foreach bdir,$(build_dir),$(eval $(call make-goal,$(bdir))))


# documentation
doc:
	doxygen

# installation
prefix		:= /usr
header		:= $(src_prefix)/core/tinkervision.h \
		   $(src_prefix)/core/tinkervision_defines.h \
                   $(src_prefix)/interface/module.hh \
                   $(src_prefix)/interface/parameter.hh \
                   $(src_prefix)/interface/result.hh \
		   $(src_prefix)/core/exceptions.hh \
                   $(src_prefix)/imaging/image.hh \
		   $(src_prefix)/core/logger.hh

install: $(build_prefix)/$(output)
	install -m 544 $(build_prefix)/$(output) $(prefix)/lib/$(output)
	mkdir -p $(prefix)/include/tinkervision/
	install -m 544 $(header) $(prefix)/include/tinkervision/

.PHONY: install

cc		:= g++

ifdef DEBUG
	ccflags := -Wall -Werror -pedantic -g -O0 -std=c++11 -fPIC -DDEBUG
else
	ccflags := -Wall -Werror -pedantic -O3 -std=c++11 -fPIC -DWITH_LOGGER
endif

# structure
parts		:= core imaging debug interface
build_prefix	:= build
build_dir	:= $(addprefix $(build_prefix)/,$(parts))
src_prefix	:= src/lib
src_dir	:= $(addprefix $(src_prefix)/,$(parts))

libs		:= -lstdc++ -lv4l2 -lm
inc		:= $(addprefix -I./$(src_prefix)/,$(parts)) $(OCV_inc)
ifneq ($(or $(WITH_OPENCV_CAM),$(DEBUG)),)
	libs	+= /usr/local/lib/libopencv_highgui.so \
		   /usr/local/lib/libopencv_imgproc.so \
		   /usr/local/lib/libopencv_video.so \
		   -lrt -lpthread -ldl
	inc	+= -I/usr/local/include/opencv -I/usr/local/include
endif

ldflags	:= $(libs) -rdynamic

# files
SRC		:= $(foreach sdir,$(src_dir),$(wildcard $(sdir)/*.cc))
ifdef DEBUG
	obj	:= $(patsubst $(src_prefix)/%.cc,build/%_dbg.o,$(SRC))
	output	:= libtinkervision_dbg.so
else
	obj	:= $(patsubst $(src_prefix)/%.cc,build/%.o,$(SRC))
	output	:= libtinkervision.so
endif


all: directories lib

.PHONY: clean

# search paths
vpath %.cc $(src_dir)

# generates targets, called at below
define make-goal
ifdef DEBUG
$1/%_dbg.o: %.cc
	$(cc) $(ccflags) -c $$< -o $$@ $(inc)
else
$1/%.o: %.cc
	$(cc) $(ccflags) -c $$< -o $$@ $(inc)
endif
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
prefix		:= /usr/local
header		:= $(src_prefix)/core/tinkervision.h \
		   $(src_prefix)/core/tinkervision_defines.h \
                   $(src_prefix)/interface/tv_module.hh \
		   $(src_prefix)/core/exceptions.hh \
                   $(src_prefix)/imaging/image.hh \
		   $(src_prefix)/core/logger.hh

install: $(build_prefix)/$(output)
	install -m 544 $(build_prefix)/$(output) $(prefix)/lib/$(output)
	mkdir -p $(prefix)/include/tinkervision/
	install -m 544 $(header) $(prefix)/include/tinkervision/

.PHONY: install

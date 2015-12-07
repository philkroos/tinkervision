cc		:= g++
ccflags	:= -Wall -Werror -pedantic -shared -std=c++14 -fpic -DWITH_LOGGER

ifdef OCVC
	ccflags += -DWITH_OPENCV_CAM
endif

ifdef DEBUG
	ccflags += -g -O0 -DDEBUG
else
	ccflags += -O3
endif

ifdef DEFAULT_CALL
	ccflags += -DDEFAULT_CALL
endif

ifdef USER_PREFIX
	usr_prefix = $(USER_PREFIX)
else
	usr_prefix = $(HOME)/tv
endif

# structure
parts		:= core tools imaging debug interface
build_prefix	:= build
build_dir	:= $(addprefix $(build_prefix)/,$(parts))
src_prefix	:= src/lib
src_dir	:= $(addprefix $(src_prefix)/,$(parts))

libs		:= -lstdc++ -lv4l2 -lm -lpython2.7
inc		:= $(addprefix -I./$(src_prefix)/,$(parts)) \
		   $(OCV_inc) \
		   -I/usr/include/python2.7
ifdef OCVC
	libs	+= `pkg-config --libs opencv` \
		   -lrt -lpthread -ldl
	inc	+= -I/usr/local/include/opencv -I/usr/local/include
endif

ldflags	:= -L/usr/lib/python2.7 $(libs) -rdynamic

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
	$(cc) $(ccflags) -c $$< -o $$@ $(inc) -DUSR_PREFIX=\"$(usr_prefix)\"
endef

# setup directory structure
$(build_dir):
	@mkdir -p $@

directories: $(build_dir) user_paths

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

# set up the user paths
user_paths:
	mkdir -p $(usr_prefix)/lib \
		 $(usr_prefix)/data \
		 $(usr_prefix)/scripts

# installation
prefix		:= /usr
header		:= $(src_prefix)/core/tinkervision.h \
		   $(src_prefix)/core/tinkervision_defines.h \
		   $(src_prefix)/interface/module.hh \
		   $(src_prefix)/interface/environment.hh \
		   $(src_prefix)/interface/parameter.hh \
		   $(src_prefix)/interface/result.hh \
		   $(src_prefix)/tools/filesystem.hh \
		   $(src_prefix)/core/exceptions.hh \
		   $(src_prefix)/core/logger.hh \
		   $(src_prefix)/core/python_context.hh \
		   $(src_prefix)/interface/image.hh

install: $(build_prefix)/$(output)
	install -m 544 $(build_prefix)/$(output) $(prefix)/lib/$(output)
	mkdir -p $(prefix)/include/tinkervision/
	install -m 544 $(header) $(prefix)/include/tinkervision/

.PHONY: install

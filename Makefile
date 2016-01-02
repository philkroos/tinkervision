cc		:= g++
ccflags	:= -Wall -Werror -pedantic -std=c++14 -fPIC

ifndef NOPY
	ccflags += -DWITH_PYTHON
endif

ifndef NOLOG
	ccflags += -DWITH_LOGGER
endif

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
parts		:= core tools imaging interface

ifndef NOLOG
	parts	+= logger
endif

ifndef NOPY
	parts	+= python
endif

build_prefix	:= build
build_dir	:= $(addprefix $(build_prefix)/,$(parts))
src_prefix	:= src/lib
src_dir	:= $(addprefix $(src_prefix)/,$(parts))

libs		:= -lstdc++ -lv4l2 -lm
inc		:= -I$(src_prefix) $(addprefix -I./$(src_prefix)/,$(parts))
# files
ifndef NOPY
	libs	+= -L/usr/lib/python2.7 -lpython2.7
	inc	+= -I/usr/include/python2.7
endif

ifdef OCVC
	libs	+= `pkg-config --libs opencv` \
		   -lrt -lpthread -ldl
	inc	+= -I/usr/local/include/opencv -I/usr/local/include
endif

ldflags	:= $(libs) -rdynamic -shared

# files
src		:= $(foreach sdir,$(src_dir),$(wildcard $(sdir)/*.cc))
obj		:= $(patsubst $(src_prefix)/%.cc,$(build_prefix)/%.o,$(src))
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
	$(cc) -o $(build_prefix)/$(output) $(obj) $(ldflags)
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
		   $(src_prefix)/interface/image.hh

ifndef NOPY
	header += $(src_prefix)/python/python_context.hh
endif

ifndef NOLOG
	header += $(src_prefix)/logger/logger.hh
endif


install: $(build_prefix)/$(output)
	install -m 544 $(build_prefix)/$(output) $(prefix)/lib/$(output)
	mkdir -p $(prefix)/include/tinkervision/
	install -m 544 $(header) $(prefix)/include/tinkervision/

remove:
	rm -rf $(prefix)/lib/$(output)
	rm -rf $(prefix)/include/tinkervision/

.PHONY: clean install remove

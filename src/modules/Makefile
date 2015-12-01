MODULES	:= colormatch grayfilter dummy motiondetect snapshot downscale
HERE		:= $(shell pwd)

ifndef PRE
	PRE = /usr/lib/tinkervision
endif

DEBUG ?= no

all:
	@for module in $(MODULES); do \
		cd $$module && DEBUG=$(DEBUG) make && cd ..; \
	done

clean:
	@for dir in $(MODULES); do \
		cd $$dir && make clean && cd ..; \
	done

install:
	export PRE
	@for dir in $(MODULES); do \
		make -C $(HERE)/$$dir install; \
	done

.phony: clean install

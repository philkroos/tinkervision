MODULES	:= grayfilter dummy colormatch motiondetect snapshot downscale

all:
	@for module in $(MODULES); do \
		cd $$module && make && cd ..; \
	done

clean:
	@for dir in $(MODULES); do \
		cd $$dir && make clean && cd ..; \
	done

install:
	@for dir in $(MODULES); do \
		cd $$dir && sudo make install && cd ..; \
	done

.phony: clean install

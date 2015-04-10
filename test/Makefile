COLORTRACK	:= colormatch
STREAM		:= stream
CONVERT	:= convert
RECORD		:= record
MOTIONDETECT	:= motiondetect
CHAIN		:= chain

ALL		:= $(COLORTRACK) $(STREAM) $(CONVERT) \
		   $(RECORD) $(MOTIONDETECT) $(CHAIN)

all:
	@for test in $(ALL); do \
		cd $$test && make && cd ..; \
	done

clean:
	@for test in $(ALL); do \
		cd $$test && make clean && cd ..; \
	done

COLORTRACK	:= colormatch
STREAM		:= stream
CONVERT	:= convert
RECORD		:= record
MOTIONDETECT	:= motiondetect

ALL		:= $(COLORTRACK) $(STREAM) $(CONVERT) $(RECORD) $(MOTIONDETECT)

all:
	@for test in $(ALL); do \
		cd $$test && make && cd ..; \
	done

clean:
	@for test in $(ALL); do \
		cd $$test && make clean && cd ..; \
	done

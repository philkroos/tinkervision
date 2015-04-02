COLORTRACK	:= colormatch
STREAM		:= stream
CONVERT	:= convert
RECORD		:= record

ALL		:= $(COLORTRACK) $(STREAM) $(CONVERT) $(RECORD)

all:
	@for test in $(ALL); do \
		cd $$test && make && cd ..; \
	done

clean:
	@for test in $(ALL); do \
		cd $$test && make clean && cd ..; \
	done

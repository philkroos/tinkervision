COLORTRACK := colortrack
STREAM := stream
CONVERT := convert

ALL := $(COLORTRACK) $(STREAM) $(CONVERT)

all:
	@for test in $(ALL); do \
		cd $$test && make && cd ..; \
	done

clean:
	@for test in $(ALL); do \
		cd $$test && make clean && cd ..; \
	done

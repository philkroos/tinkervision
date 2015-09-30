COLORTRACK	:= colormatch
STREAM		:= stream
CONVERT	:= convert
SNAPSHOT	:= snapshot
MOTIONDETECT	:= motiondetect
SCENES		:= scenes
GENERAL	:= general
FS		:= filesystem
ML		:= moduleloader

ALL		:= $(COLORTRACK) $(STREAM) $(CONVERT) \
		   $(SNAPSHOT) $(MOTIONDETECT) \
		   $(GENERAL) $(SCENES) $(ML) $(FS)

all:
	@for test in $(ALL); do \
		cd $$test && make && cd ..; \
	done

clean:
	@for test in $(ALL); do \
		cd $$test && make clean && cd ..; \
	done

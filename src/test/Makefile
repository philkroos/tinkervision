COLORTRACK	:= colormatch
#STREAM		:= stream
CONVERT	:= convert
SNAPSHOT	:= snapshot
MOTIONDETECT	:= motiondetect
SCENES		:= scenes
GENERAL	:= general
FS		:= filesystem
ML		:= moduleloader
DW		:= dirwatch

ALL		:= $(COLORTRACK) $(CONVERT) \
		   $(SNAPSHOT) $(MOTIONDETECT) \
		   $(GENERAL) $(SCENES) $(ML) $(FS) $(DW)# $(STREAM)
all:
	@for test in $(ALL); do \
		cd $$test && make && cd ..; \
	done

clean:
	@for test in $(ALL); do \
		cd $$test && make clean && cd ..; \
	done

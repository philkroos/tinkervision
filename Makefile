SUBDIRS = src test

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

test: src
	cd test && make test

clean:
	cd test && make clean
	cd src && make clean
	rm -rf bin

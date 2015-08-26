all:
	cd python-bindings-generator/python && \
	sudo rm -rf unzipped && \
	python generate_python_bindings.py && \
	python generate_python_zip.py && \
	mkdir unzipped && \
	mv *.zip unzipped && \
	cd unzipped && \
	unzip *.zip && \
	cd source && \
	sudo python setup.py install

clean:
	echo "Please clean the bindings manually"

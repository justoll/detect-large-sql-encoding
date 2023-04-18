PYTHON = python3  
SETUP = setup.py
TARGET = detectsqlencoding.so  # target lib
TARGET2 = ../testDetectSQLEncoding/detectsqlencoding.so  # target lib
BUILDDIR  =  build
WHEELCMD  =  bdist_wheel
BUILDTARGET = build/lib.linux-x86_64-3.8/detectsqlencoding.cpython-38-x86_64-linux-gnu.so
WHEELSOURCE = dist/*.whl


.PHONY: all
all: 
	$(PYTHON) $(SETUP) $(BUILDDIR)

.PHONY: clean
clean:
	rm -f $(TARGET) 
	rm -R $(BUILDDIR) 
	
.PHONY: install
install:
	cp  $(BUILDTARGET) $(TARGET2) 
	mv  $(BUILDTARGET) $(TARGET) 

wheel:
	$(PYTHON) $(SETUP) $(WHEELCMD)

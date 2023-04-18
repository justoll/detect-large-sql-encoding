ICUDIR = ../icu
LIBDETECTSQLENCODINGDIR = ../libDetectSQLEncoding
ENCADIR = ../libDetectSQLEncoding/enca

TARGET_BINARY = detectSQLEncoding  
SRCS = src/main.cpp
OBJS = $(SRCS:.cpp=.o)
RM = rm -f   # rm command

detectSQLEncoding: src/main.o
	g++ -L$(LIBDETECTSQLENCODINGDIR)/lib  -L$(ICUDIR)/lib -o $(TARGET_BINARY) src/main.o  -lm -l:libDetectSQLEncoding.so -l:libicudata.so.71.1  -l:libicui18n.so.71.1 -l:libicuio.so.71.1 -l:libicutest.so.71.1 -l:libicutu.so.71.1 -l:libicuuc.so.71.1 -l:libenca.so 

$(OBJS): $(SRCS)
	g++ -c -Wall -Wextra -O2  $(SRCS) -o $(OBJS) -I $(ICUDIR)/include -I $(LIBDETECTSQLENCODINGDIR)/include

.PHONY: clean
clean:
	-${RM} ${TARGET_BINARY} ${OBJS} $(SRCS:.cpp=.d)
	

# Makefile for Basler Pylon sample program
.PHONY			: all clean install

# The program to build
NAME			:= ocrs

# Build tools and flags
CXX				?= g++
LD         := $(CXX)
CXXFLAGS   := #e.g., CXXFLAGS=-g -O0 for debugging

COPENCV     := $(shell pkg-config --cflags opencv)
CTESSERACT  := $(shell pkg-config --cflags tesseract)
CZBAR  := $(shell pkg-config --cflags zbar)
CBOOST  :=
CMYSQL 		  := $(shell mysql_config --cflags)

LOPENCV     := $(shell pkg-config --libs opencv)
LTESSERACT  := $(shell pkg-config --libs tesseract)
LZBAR  := $(shell pkg-config --libs zbar)
LBOOST  := -lboost_system -lboost_regex -lboost_filesystem
LMYSQL 		  := $(shell mysql_config --libs)

# Rules for building
all				: $(NAME)

$(NAME)			: $(NAME).o ImageRecognize.o ExtractAddress.o glob_posix.o
	$(LD) -o $@ $^ $(LOPENCV) $(LTESSERACT) $(LMYSQL) $(LZBAR) $(LBOOST)

$(NAME).o: $(NAME).cpp ImageRecognize.cpp ExtractAddress.cpp glob_posix.cpp
	$(CXX) $(COPENCV) $(CTESSERACT) $(CMYSQL) $(CZBAR) $(CBOOST) -c -o $@ -std=c++11 $<

ImageRecognize.o: ImageRecognize.cpp
	$(CXX) $(COPENCV) $(CTESSERACT) $(CMYSQL) $(CZBAR) $(CBOOST) -c -o $@ -std=c++11 $<

ExtractAddress.o: ExtractAddress.cpp
	$(CXX) $(COPENCV) $(CTESSERACT) $(CMYSQL) $(CZBAR) $(CBOOST) -c -o $@ -std=c++11 $<

glob_posix.o: glob_posix.cpp
	$(CXX) $(COPENCV) $(CTESSERACT) $(CMYSQL) $(CZBAR) $(CBOOST) -c -o $@ -std=c++11 $<

clean			:
	$(RM) $(NAME).o $(NAME)

install		: $(NAME)
	install -m 0755 $(NAME) /usr/bin/

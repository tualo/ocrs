# Makefile for Basler Pylon sample program
.PHONY			: all clean install

# The program to build
NAME			:= ocrs

# Installation directories for GenICam and Pylon
PYLON_ROOT		?= /opt/pylon5
GENICAM_ROOT	?= $(PYLON_ROOT)/genicam

# Build tools and flags
CXX				?= g++
LD         := $(CXX)
CXXFLAGS   := #e.g., CXXFLAGS=-g -O0 for debugging

CMYSQL 		  := $(shell mysql_config --cflags)
COPENCV     := $(shell pkg-config --cflags opencv)
CTESSERACT  := $(shell pkg-config --cflags tesseract)

LOPENCV     := $(shell pkg-config --libs opencv)
LTESSERACT  := $(shell pkg-config --libs tesseract)
LMYSQL 		  := $(shell mysql_config --libs)

# Rules for building
all				: $(NAME)

$(NAME)			: $(NAME).o
	$(LD) -o $@ $^ $(LOPENCV) $(LTESSERACT) $(LMYSQL)

$(NAME).o: $(NAME).cpp
	$(CXX) $(COPENCV) $(CTESSERACT) $(CMYSQL) -c -o $@ $<

clean			:
	$(RM) $(NAME).o $(NAME)

install		: $(NAME)
	install -m 0755 $(NAME) /opt/ocrs/

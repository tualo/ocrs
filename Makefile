# Makefile
.PHONY			: all clean install

# The program to build
NAME			:= ocrs
OUTPUT   	:= $(NAME)



# Build tools and flags
CXX				?= g++

LD         := $(CXX)
CPPFLAGS   := $(shell mysql_config --cflags) $(shell pkg-config zbar --cflags) $(shell pkg-config opencv --cflags) $(shell pkg-config tesseract --cflags)
CXXFLAGS   := -Wall -std=c++14  #e.g., CXXFLAGS=-g -O0 for debugging
LDFLAGS    := $(shell mysql_config --libs_r)
LDLIBS     := $(shell mysql_config --libs) $(shell pkg-config zbar --cflags --libs) $(shell pkg-config opencv --libs)  $(shell pkg-config tesseract --libs) -lboost_system -lboost_regex -lboost_filesystem -lboost_thread-mt -lboost_chrono -lboost_iostreams -lboost_atomic-mt -lboost_date_time -lpthread


# Rules for building
all				: $(NAME)

$(NAME)			: $(NAME).o
	$(LD) $(LDFLAGS) -o $@ ExtractAddress.o glob_posix.o ImageRecognize.o ocrs.o $(LDLIBS)


$(NAME).o: $(SOURCES) $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./glob_posix.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./ExtractAddress.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./ImageRecognize.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./ocrs.cpp

clean			:
	$(RM) $(NAME).o $(NAME)
	$(RM) ExtractAddress.o
	$(RM) glob_posix.o
	$(RM) ImageRecognize.o

cleanobj			:
	$(RM) $(NAME).o
	$(RM) ExtractAddress.o
	$(RM) glob_posix.o
	$(RM) ImageRecognize.o


install		: $(NAME)
	install -m 0755 $(NAME) /opt/ocrs/

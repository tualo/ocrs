# Makefile
.PHONY			: all clean install

# The program to build
NAME			:= ocrsex
OUTPUT   	:= $(NAME)


#TESSERACT_CPPFLAGS := $(shell pkg-config tesseract --cflags)
TESSERACT_CPPFLAGS := $(shell echo "-I/Users/thomashoffmann/Documents/Projects/cpp/tesseract/api -I/usr/local/Cellar/leptonica/1.74.1/include/leptonica")
#TESSERACT_LDLIBS := $(shell pkg-config tesseract --libs)
TESSERACT_LDLIBS := $(shell echo "-L/Users/thomashoffmann/Documents/Projects/cpp/tesseract/api/.libs -ltesseract")

OS := $(shell uname)
ifeq ($(OS),Darwin)
  # Run MacOS commands
TESSERACT_CPPFLAGS := $(shell echo "-I/Users/thomashoffmann/Documents/Projects/cpp/tesseract/api -I/usr/local/Cellar/leptonica/1.74.1/include/leptonica")
TESSERACT_LDLIBS := $(shell echo "-L/Users/thomashoffmann/Documents/Projects/cpp/tesseract/api/.libs -ltesseract")

else
  # check for Linux and run other commands
TESSERACT_CPPFLAGS := $(shell pkg-config tesseract --cflags)
TESSERACT_LDLIBS := $(shell pkg-config tesseract --libs)

endif

# Build tools and flags
CXX				?= g++

LD         := $(CXX)
CPPFLAGS   := $(shell mysql_config --cflags) $(shell pkg-config zbar --cflags) $(shell pkg-config opencv --cflags) $(shell pkg-config tesseract --cflags) $(TESSERACT_CPPFLAGS)
CXXFLAGS   := -Wall -std=c++14 -O3  #e.g., CXXFLAGS=-g -O0 for debugging
LDFLAGS    := $(shell mysql_config --libs_r)

ifeq ($(OS),Darwin)
LDLIBS     := $(shell mysql_config --libs) $(shell pkg-config zbar --cflags --libs) $(shell pkg-config opencv --libs) $(TESSERACT_LDLIBS) -lboost_system -lboost_regex -lboost_filesystem -lboost_thread-mt -lboost_chrono -lboost_iostreams -lboost_atomic-mt -lboost_date_time -lpthread
LDLIBS     := -L/usr/local/Cellar/mariadb/10.3.8/lib  -lmariadb -lz -liconv $(shell pkg-config zbar --cflags --libs) $(shell pkg-config opencv --libs) $(TESSERACT_LDLIBS) -lboost_system -lboost_regex -lboost_filesystem -lboost_thread-mt -lboost_chrono -lboost_iostreams -lboost_atomic-mt -lboost_date_time -lpthread
LDFLAGS    := -L/usr/local/Cellar/mariadb/10.3.8/lib  -lmariadb -lz -liconv

else
LDLIBS     := $(shell mysql_config --libs) $(shell pkg-config zbar --cflags --libs) $(shell pkg-config opencv --libs) $(TESSERACT_LDLIBS) -lboost_system -lboost_regex -lboost_filesystem -lboost_thread -lboost_chrono -lboost_iostreams -lboost_atomic -lboost_date_time -lpthread
endif

# Rules for building
all				: $(NAME)

$(NAME)			: $(NAME).o
	$(LD) $(LDFLAGS) -o $@ RegionOfInterest.o ExtractAddress.o glob_posix.o Barcode.o FindCodes.o ImageRecognizeEx.o ocrsex.o  $(LDLIBS)

SOURCES := $(shell find . -name '*.cpp')
HEADERS := $(shell find . -name '*.h')


$(NAME).o: $(SOURCES) $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./RegionOfInterest.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./glob_posix.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./ExtractAddress.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./recognize/imagecodes/Barcode.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./recognize/imagecodes/FindCodes.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./ImageRecognizeEx.cpp
	
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./ocrsex.cpp

clean			:
	$(RM) $(NAME).o $(NAME)
	$(RM) RegionOfInterest.o
	$(RM) ExtractAddress.o
	$(RM) glob_posix.o
	$(RM) ImageRecognizeEx.o
	$(RM) test_reg.o


install		: $(NAME)
	echo "not implemented"

CXX=g++
CXXFLAGS+=-g -I. -std=c++11
OBJ=commandInterface.o
TARGET=libci.a
EXETEST=citest
AR=ar

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $^ -o $@

all: $(OBJ) $(TARGET) $(EXETEST)

$(TARGET): $(OBJ)
	$(AR) rv $(TARGET) $(OBJ)

$(EXETEST): $(EXETEST).cpp
	sudo make install
	$(CXX) $(CXXFLAGS) $^ -o $@ -lci

install:
	@install -C $(TARGET) /usr/local/lib
	@install -C commandInterface.hpp /usr/local/include

clean:
	rm -f $(TARGET) $(OBJ) $(EXETEST) *~

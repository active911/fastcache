#GEN_SRC := tests.cpp example.cpp
DEP_HEAD := Fastcache.h
#gGEN_OBJ := $(patsubst %.cpp,%.o, $(GEN_SRC))

#THRIFT_DIR := /usr/local/include/thrift
#INC := -I$(THRIFT_DIR) -Igen-cpp/ -Iinclude/ -Iinih/ -Iinih/cpp/

all: tests example

%.o: %.cpp $(DEP_HEAD)
	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H $(INC) -c $< -o $@

tests: tests.o 
	$(CXX) $^ -o $@ -L/usr/local/lib 

example: example.o 
	$(CXX) $^ -o $@ -L/usr/local/lib 

clean:
	$(RM) *.o  tests example
#GEN_SRC := tests.cpp example.cpp
DEP_HEAD := Fastcache.h
#gGEN_OBJ := $(patsubst %.cpp,%.o, $(GEN_SRC))

#THRIFT_DIR := /usr/local/include/thrift
#INC := -I$(THRIFT_DIR) -Igen-cpp/ -Iinclude/ -Iinih/ -Iinih/cpp/

all: tests example

%.o: %.cpp $(DEP_HEAD)
	$(CXX) -Wall $(INC) -c $< -o $@

tests: test1 test2 test3

test%: test/test%.cpp
	$(CXX) $^ -o test/$@ -L/usr/local/lib -lboost_thread

example: example.o 
	$(CXX) $^ -o $@ -L/usr/local/lib 

clean:
	$(RM) *.o  test/*.o test/test1 test/test2 test/test3 example


test_all: cb_test
	./cb_test

cb_test: cb_build cb_test.o 
	g++ -o cb_test cb_test.o case_base.o

cb_test.o: cb_test.cpp case_base.h
	g++ -g -c -o cb_test.o cb_test.cpp

compile_all: cb_build

cb_build: case_base.o

case_base.o: case_base.cpp case_base.h
	g++ -g -c -o case_base.o case_base.cpp

clean:
	rm -f *.o cb_test

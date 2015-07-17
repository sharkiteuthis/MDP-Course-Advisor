run: kb_test
	./kb_test

kb_test: compile_all kb_test.o kb.h
	g++ -o kb_test kb_test.o kb.o

compile_all: kb.o kb.h

kb_test.o: kb_test.cpp
	g++ -o kb_test.o -g -c kb_test.cpp

kb.o: kb.cpp kb.h
	g++ -o kb.o -g -c kb.cpp

clean:
	rm -rf *.o kb_test

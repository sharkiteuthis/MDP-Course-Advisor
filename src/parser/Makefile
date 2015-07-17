test_all: parsers_test
	./parsers_test

parsers_test: parsers_build parsers_test.o 
	g++ -o parsers_test parsers_test.o policy_parser.o model_parser.o tokenizer.o

compile_all: parsers_build

parsers_build: tokenizer.o model_parser.o policy_parser.o

parsers_test.o: parsers_test.cpp model_parser.h policy_parser.h
	g++ -g -c -o parsers_test.o parsers_test.cpp

tokenizer.o: tokenizer.cpp
	g++ -g -c -o tokenizer.o tokenizer.cpp

model_parser.o: model_parser.cpp tokenizer.h parser_types.h
	g++ -g -c -o model_parser.o model_parser.cpp

policy_parser.o: policy_parser.cpp tokenizer.h parser_types.h
	g++ -g -c -o policy_parser.o policy_parser.cpp

clean:
	rm -f *.o parsers_test

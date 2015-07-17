run: gui
	./gui

gui: case_base parsers kb gui.o menu_maker.o mdp_math.o explainer.o
	g++ -o gui gui.o menu_maker.o explainer.o mdp_math.o kb/kb.o parser/model_parser.o parser/policy_parser.o parser/tokenizer.o case_base/case_base.o -lncurses -lmenu

gui.o: gui.cpp menu_maker.h mdp_math.h parser/model_parser.h parser/policy_parser.h explainer.h
	g++ -c -g gui.cpp

menu_maker.o: menu_maker.cpp
	g++ -c -g menu_maker.cpp

mdp_math.o: mdp_math.cpp parser/model_parser.h parser/policy_parser.h
	g++ -c -g mdp_math.cpp

explainer.o: explainer.cpp mdp_math.h parser/model_parser.h parser/policy_parser.h case_base/case_base.h kb/kb.h
	g++ -c -g explainer.cpp

.PHONY: case_base parsers kb clean

case_base:
	cd case_base && make compile_all && cd ..

parsers:
	cd parser && make compile_all && cd ..

kb:
	cd kb && make compile_all && cd ..

clean:
	rm -rf *.o gui
	cd parser && make clean && cd ..

debug:
	g++-4.6 -std=c++0x parser.cpp -o parser.exec -lboost_regex -ggdb
release:
	g++-4.6 -std=c++0x parser.cpp -o parser.exec -lboost_regex

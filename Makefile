main: main.cpp SpaceGame.h SpaceGame.cpp 
	clang++ -std=c++17 main.cpp SpaceGame.cpp Config.cpp -lncurses -lpqxx -lpq -o main

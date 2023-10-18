#include "SpaceGame.h"
#include "Config.h"
#include <ncurses.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <time.h>
#include <random>
#include <pqxx/pqxx>

#define EXIT_SUCCESS 0

int main() {
    SpaceGame newGame;
    newGame.game();
    return EXIT_SUCCESS;
}

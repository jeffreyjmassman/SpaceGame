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

using std::cout; using std::endl; using std::string; using std::vector;

int main() {

    SpaceGame newGame;
    newGame.game();
    
    // string dbconnect = "host=localhost port=5432 dbname=games user"
    // "=postgres password=YsBt#92Ah1234 sslmode=prefer connect_timeout=10";
    // pqxx::connection C(dbconnect);
    // auto dbconnect = "dbname = games host = localhost port = 5432 user = postgres password = YsBt#92Ah1234";
    
    // Config config;

    // pqxx::connection C(
    //     "dbname = " + config.DB_NAME +
    //     "host = localhost "
    //     "port = 5433 "
    //     "user = " + config.DB_USERNAME +
    //     "password = " + config.DB_PASSWORD
    // );

    // pqxx::work W(C);

    // int accurate = 78;
    // double accurateDouble = accurate / (double) 100;
    // string accurateString = std::to_string(accurateDouble);

    // W.exec("INSERT INTO test (name, score, accuracy) VALUES ('JJM', 22, " + accurateString + ")");
    // W.commit();
    // for (int i = 0; i < 3; i++) {
    //     pqxx::work W(C);
    //     pqxx::result r = W.exec("SELECT COUNT(*) FROM test");
    //     W.commit();
    //     cout << stoi(to_string(r[0][0])) << "\n";
    // }
    // cout << endl;
    // for (auto const & row : r) {
    //     for (auto const & field : row) {
    //         cout << field.c_str() << "\n";
    //     }
    // }
    // cout << endl;


    // int i = 0;
    // initscr();
    // while(i < 10) {
    //     int ch;
    //     ch = getch();
    //     mvprintw(20,20, "%d", ch);
    //     refresh();
    //     i++;  
    // }
    // endwin();

    return EXIT_SUCCESS;
}
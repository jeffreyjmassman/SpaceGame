# SpaceGame
An arcade-inspired console game written in C++.

This game is an arcade-style shooter game, similar to space invaders and asteroids. Enemies will spawn on the right half of the screen and attempt to make their way towards the left. Your job is to stop them. When you destroy enough enemies, you will advance to the next level. You get +1 score for every regular enemy destroyed by projectiles, and +2 for enemies destroyed by powerups. On that note, you can collect powerups and lives periodically (one chance to spawn per level for each of them). Every four levels is a boss level. The boss fires projectiles and homing missiles at you, and the boss creates a barrier that your projectiles and powerups cannot penetrate. You must find missiles around the screen that can pass through the barrier, and use them against the boss. On boss levels. lives and powerups may spawn multiple times.

The controls are W, A, S, D to move, SPACE to shoot, K for powerups, M for missiles, and P to pause the game.

The game was created using standard C++ along with the ncurses library. It is compiled using clang, and is designed to run on Mac OSX. Sounds are handled using the regular terminal and afplay. I have also setup a private PostgreSQL server and database, and interface with it using libpqxx in order to store player scores and other data. This way, scores are saved between game runs and aren't erased after program termination. The database stores other data as well, which could be used for analysis down the line.

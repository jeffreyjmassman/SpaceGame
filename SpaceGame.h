#include <vector>
#include <string>
#include <random>
#include "Config.h"
#include <pqxx/pqxx>

using std::vector; using std::string;

class SpaceGame {

    private:

        /**
         * Random number generators; unbiased.
        */
        std::mt19937 rng;
        std::uniform_int_distribution<int> enemy_y_gen;
        std::uniform_int_distribution<int> spawn_gen;
        std::uniform_int_distribution<int> powerupLifex_gen;
        std::uniform_int_distribution<int> powerupLifey_gen;
        std::uniform_int_distribution<int> powerupLifexBossLevel_gen;

        /**
         * List of projectiles by their coordinates in the frame.
        */
        vector<vector<int> > projectiles;

        /**
         * List of enemies by their coordinates in the frame.
        */
        vector<vector<int> > enemies;

        /**
         * List of powerup projectiles.
        */
        vector<vector<int> > powerupProjectiles;

        /**
         * Player score, i.e. number of enemies shot down.
        */
        int score;

        /**
         * Coordinates of the rightmost end of the player's ship.
        */
        int shipx;
        int shipy;

        /**
         * Stores the mid height and width of the terminal.
        */
       int midh;
       int midw;

        /**
         * indicates if the current game is over or not.
        */
        bool gameOver;

        /**
         * The names of the three highest scorers, and their scores.
        */
        vector<string> names;
        vector<int> highScores;

        /**
         * Tracks total number of shots fired from the player, as well
         * as the number of enemy hits from projectiles (not including 
         * hits from a powerup).
        */

        int shotsFired;
        int numHits;

        /**
         * The score of the current level.
        */
        int levelScore;

        /**
         * Controls the enemy speed.
        */
        int enemySpeed;

        /**
         * boolean flag indicating whether the player would like to
         * keep playing or exit the game.
        */
        bool keepPlaying;

        /**
         * Number of player lives
        */
        int lives;

        /**
         * The current level
        */
        int level;

        /**
         * Controls the chance of enemies spawning per frame.
        */
        double threshold;

        /**
         * Stores the location of a powerup, and a boolean flag
         * indicating whether a powerup spawned for the level. and
         * if a powerup currently exists on the map, and if the player
         * currently holds a powerup.
        */
        vector<int> powerup;
        bool spawnedPowerup;
        bool powerupExists;
        bool playerHasPowerup;

        /**
         * Stores the location of a life, and a boolean flag
         * indicating whether a life spawned for the level and
         * if a life currently exists on the map.
        */
        vector<int> life;
        bool spawnedLife;
        bool lifeExists;

        /**
         * Used for frame animations.
        */
        int counter;

        /**
         * Determines whether to draw powerup or life.
        */
        bool drawPowerupLife;

        /**
         * A vector that stores dead enemies for explosion effect.
        */
        vector<vector<int> > deadEnemies;

        /**
         * Indicates if the current level is a boss level.
        */
        bool isBossLevel;

        /**
         * Vector contains information on the boss, including coordinates
         * and health. bossMissile is a boss missile projectile, the
         * vector contains boss projectile information, and
         * the boolean indicates if the boss shot a missile.
        */
        vector<int> boss;
        vector<int> bossMissile;
        vector<vector<int> > bossProjectiles;
        bool bossShotMissile;

        /**
         * Missile collectible, missile projectile, a bool
         * indicating whether the player currently holds a missile,
         * and a bool indicating if a missile collectible currently exists.
        */
        vector<int> missileCollectible;
        vector<int> missile;
        bool playerHasMissile;
        bool missileExists;

        /**
         * ints used to animate player and boss damage, as well as provide
         * the player with some protection following damage taken; the
         * bools indicate if the ship / player should be drawn on a given frame.
        */
        int iframes;
        int bossDrawFrames;
        bool shipDraw;
        bool bossDraw;

        /**
         * Handles the repeating of boss music; roughly every 1:26
        */
       int bossMusicCounter;

       /**
        * Whether the game is in online or offline mode. Online mode = game will establish a connection
        * to the postgreSQL score database and read / write. Offline mode = game will use the default
        * behavior of only recording the scores of the current session. The second bool is
        * used for keyroute purposes. The config variable contains database credentials.
       */
       bool online;
       bool gameStarted;
       Config config;

       /**
        * For querying the database.
       */
       pqxx::connection * C;

       /**
        * Some variables used for data collection in online mode.
       */
       int livesUsed;
       int powerupsUsed;
       int enemiesKilled;
       int bossesKilled;
       int accuracy;

    public:

        /**
        * Constructor.
        */
        SpaceGame();

        /**
         * Function that actually runs the game from a SpaceGame instance.
        */
        void game();

    private:

        /**
         * Updates the frames dispalyed on the console.
        */
        void frameUpdate();

        /**
         * Routes the pressed key to the correct method.
        */
        void keyRoute(int ch);

        /**
         * Updates the location of projectiles fired from the player on regular intervals
        */
        void projectileMoveUpdate();

        /**
         * Updates the location of the player's ship.
        */
        void shipMoveUpdate(int ch);

        /**
         * Updates the enemy speed as score increases.
        */
        void updateEnemySpeed();

        /**
         * Updates the location of enemies.
        */
        void enemyMoveUpdate();

        /**
         * Updates location of powerup projectiles.
        */
        void powerupMoveUpdate();

        /**
         * Updates the boss location randomly.
        */
        void bossMoveUpdate(int direction);

        /**
         * Determines whether the boss will move, and what direction.
        */
        void bossMoveUpdateChance();

        /**
         * Updates the location of player-fired missiles.
        */
        void missileMoveUpdate();

        /**
         * Updates the location of boss-fired missiles.
        */
        void bossMissileMoveUpdate();

        /**
         * Updates the location of boss-fired projectiles.
        */
        void bossProjectileMoveUpdate();

        /**
         * Handles the spawning of projectiles from the player.
        */
        void spawnProjectile();

        /**
         * Handles the spawning of enemies at random y locations.
        */
        void spawnEnemy();

        /**
         * Spawns a boss on boss levels.
        */
        void spawnBoss();

        /**
         * Attempts to spawn a powerup if none has already spawned.
        */
        void spawnPowerup();

        /**
         * Detects if player collected a powerup.
        */
        void powerupCollected();

        /**
         * Despawns a powerup if not collected after awhile.
        */
        void despawnPowerup();

        /**
         * Handles when the player uses a powerup.
        */
        void usePowerup();

        /**
         * Attempts to spawn a life if none has already spawned.
        */
        void spawnLife();

        /**
         * Detects if player collected a life.
        */
        void lifeCollected();

        /**
         * Despawns a life if not collected after awhile.
        */
        void despawnLife();

        /**
         * Attempts to spawn a missle if the player has none.
        */
        void spawnMissileCollectible();

        /**
         * Detects if player collected a missle.
        */
        void missileCollected();

        /**
         * Despawns a missile if not collected after awhile.
        */
        void despawnMissileCollectible();

        /**
         * Spanws player missile if fired.
        */
        void spawnMissile();

        /**
         * Attempts to spawn boss missiles.
        */
        void spawnBossMissile();

        /**
         * Attempts to spawn boss projectiles.
        */
        void spawnBossProjectiles();

        /**
         * Detects collisions between enemies and player projectiles.
        */
        void enemyProjectileCollisionUpdate();

        /**
         * Detects collision between boss and player missiles.
        */
        void bossMissileCollisionUpdate();

        /**
         * Detects collision between enemies and player missiles.
        */
        void enemyMissileCollisionUpdate();
        
        /**
         * Detects collision between player and boss missiles.
        */
        void shipBossMissileCollisionUpdate();

        /**
         * Detects collision between player and boss projectiles.
        */
        void shipBossProjectileCollisionUpdate();

        /**
         * Detects collisions between the player and enemies.
        */
        void shipEnemyCollisionUpdate();

        /**
         * Detects collisions between powerup projectiles and enemies.
        */
        void enemyPowerupCollisionUpdate();

        /**
         * Checks whether the level needs to be updated.
        */
        void levelUpdate();

        /**
         * Animates the ship upon losing a life.
        */
        void damageAnimation();

        /**
         * Animates the boss upon losing health.
        */
        void bossDamageAnimation();

        /**
         * Animates the boss upon blowing up.
        */
        void bossExplosionAnimation();

        /**
         * Animates enemies exploding after being hit by a projectile.
        */
        void enemyExplosionAnimation(int x, int y);

        /**
         * Clears dead enemies if necessary.
        */
        void clearDeadEnemies();

        /**
         * Initializes the game state; runs at the beginning of a game.
         * Initializes locations, random seed, etc.
        */
        void initialize();

        /**
         * Prints boss to the screen.
        */
        void printBoss();

        /**
         * 'unprints' the boss.
        */
        void unprintBoss();

        /**
         * Prints the boss' health bar at the top of the screen.
        */
       void printBossHealthBar();

        /**
         * Prints the barrier during boss levels.
        */
        void printBarrier();

        /**
         * Functionality for pausing the game.
        */
        void pause();

        /**
         * Prints the start game screen.
        */
        void printStartGame();

        /**
         * Prints the SpaceGame logo.
        */
        void printLogo();

        /**
         * Prints the game rules.
        */
       void printRules();

        /**
         * The online or offline mode prompt screen.
        */
        void printOnlineMode();

        /**
         * Checks if a new top 3 high score was achieved after
         * the game is over. Note: the game only tracks the three 
         * highest scores; all others are deleted.
        */
        bool isNewHighScore();

        /**
         * Updates high scores and names upon game over.
        */
        void highScoreUpdate();

        /**
         * Prompts user to enter their initials after a game if a
         * high score was not achieved. Only for online mode.
         * This is for database purposes.
        */
        void enterInitials();

        /**
         * Prints the 'Game Over' screen to the console, and prompts user
         * to add name if a top 3 high score is reached.
        */
        void printGameOver();

        /**
         * Prints and updates the player level.
        */
        void printNewLevel();

        /**
         * Prints the scores after player quits game.
        */
        void printScores();

        /**
         * Prints the starting over screen.
        */
        void printStartOver();

        /**
         * Prints the end game screen.
        */
        void printEndGame();

        /**
         * If in online mode, connects to the database.
        */
        void establishDatabaseConnection();

        /**
         * Takes as a parameter the player initials, and returns
         * the SQL command to insert into the database.
        */
        string insertString(string name);

        /**
         * Quick absolute value function so we need not import
         * any unnecessary libraries
        */
        static int abs(int a);

        /**
         * mvprints debug message to the screen to see where execution
         * stops; used for debugging purposes only.
        */
        void debug();
};
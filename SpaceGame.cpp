//#pragma once
#include "SpaceGame.h"
#include "Config.h"
#include <ncurses.h>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include <random>
#include <pqxx/pqxx>

#define WIDTH 130
#define HEIGHT 40

using std::string; using std::vector;

SpaceGame::SpaceGame() {
    C = NULL;
    Config config;
    rng = std::mt19937(time(0));
    enemy_y_gen = std::uniform_int_distribution<int>(1, HEIGHT - 1);
    spawn_gen = std::uniform_int_distribution<int>(0, 2500);
    powerupLifex_gen = std::uniform_int_distribution<int>(1, WIDTH - 1);
    powerupLifey_gen = std::uniform_int_distribution<int>(1, HEIGHT - 1);
    powerupLifexBossLevel_gen = std::uniform_int_distribution<int>(1, 2 * WIDTH / 3 - 2);
    midh = HEIGHT / 2;
    midw = WIDTH / 2;
    shipx = WIDTH / 2;
    shipy = HEIGHT / 2;
    score = 0;
    shotsFired = 0;
    numHits = 0;
    enemySpeed = 20;
    keepPlaying = true;
    gameOver = false;
    lives = 3;
    level = 1;
    levelScore = 0;
    threshold = 0.004;
    counter = 1;
    spawnedPowerup = false;
    powerupExists = false;
    spawnedLife = false;
    lifeExists = false;
    playerHasPowerup = false;
    drawPowerupLife = false;
    isBossLevel = false;
    iframes = 0;
    bossDrawFrames = 0;
    gameStarted = false;
    livesUsed = 3;
    powerupsUsed = 0;
    enemiesKilled = 0;
    bossesKilled = 0;
    accuracy = 0;
    highScores.push_back(0); highScores.push_back(0); highScores.push_back(0); 
}

void SpaceGame::game() {
    initscr();
    curs_set(0);
    noecho();
    printStartGame();
    printRules();
    printOnlineMode();
    printNewLevel();
    nodelay(stdscr, TRUE);
    int ch;
    while(keepPlaying) {
        if (!isBossLevel) {
            if ((ch = getch()) != ERR) {
                keyRoute(ch);
            }
            powerupMoveUpdate();
            projectileMoveUpdate();
            spawnEnemy();
            if (counter % enemySpeed == 0) {
                enemyMoveUpdate();
            }
            enemyProjectileCollisionUpdate();
            shipEnemyCollisionUpdate();
            enemyPowerupCollisionUpdate();
            clearDeadEnemies();
            if (counter == 0) {
                if (powerupExists) {
                    despawnPowerup();
                }
                if (lifeExists) {
                    despawnLife();
                }
                if (!spawnedPowerup) {
                    spawnPowerup();
                }
                if (!spawnedLife) {
                    spawnLife();
                }
            }
            if (lifeExists) {
                lifeCollected();
            }
            if (powerupExists) {
                powerupCollected();
            }
            if (counter % 30 == 0) {
                drawPowerupLife = !drawPowerupLife;
            }
            if (keepPlaying) {
                frameUpdate();
            }
            counter  = (counter + 1) % 600; // change this
            if (gameOver) {
                printGameOver();
            }
            else {
                levelUpdate();
            }
            // ~60 fps, roughly
            std::this_thread::sleep_for(std::chrono::milliseconds(1000/60)); 
        }
        else {
            if ((ch = getch()) != ERR) {
                keyRoute(ch);
            }
            if (bossMusicCounter == 0) {
                system("afplay Sounds/Boss_Theme_v1.mp3 >/dev/null 2>&1 &");
            }
            powerupMoveUpdate();
            projectileMoveUpdate();
            if (boss[4] == 0) {
                if (counter % 60 == 0) {
                    bossMoveUpdateChance();
                }
            }
            bossProjectileMoveUpdate();
            if (counter % 2 == 0) {
                bossMissileMoveUpdate();
            }
            // move updates
            if (boss[4] > 0) {
                if (counter % 4 == 0) {
                    bossMoveUpdate(boss[3]);
                    boss[4] -= 1;
                }
            }
            if (counter % 4 == 0) {
                spawnEnemy();
            }
            if (counter % enemySpeed == 0) {
                enemyMoveUpdate();
            }
            enemyProjectileCollisionUpdate();
            shipEnemyCollisionUpdate();
            enemyPowerupCollisionUpdate();
            shipBossMissileCollisionUpdate();
            bossMissileCollisionUpdate();
            enemyMissileCollisionUpdate();
            shipBossProjectileCollisionUpdate();
            clearDeadEnemies();
            // collision updates, then spawn updates
            if (counter == 0) {
                if (missileExists) {
                    despawnMissileCollectible();
                }
                if (!playerHasMissile) {
                    spawnMissileCollectible();
                }
                if (lifeExists) {
                    despawnLife();
                }
                if (powerupExists) {
                    despawnPowerup();
                }
                spawnLife();
                spawnPowerup();
            }
            if (lifeExists) {
                lifeCollected();
            }
            if (powerupExists) {
                powerupCollected();
            }
            if (missileExists) {
                missileCollected();
            }
            if (counter % 30 == 0) {
                spawnBossProjectiles();
                drawPowerupLife = !drawPowerupLife;
            }
            if (counter % 120 == 0) {
                if (!bossShotMissile) {
                    spawnBossMissile();
                }
            }
            if (!missile.empty()) {
                missileMoveUpdate();
            }
            if (keepPlaying && boss[2] != 0) {
                frameUpdate();
            }
            counter  = (counter + 1) % 600;
            bossMusicCounter = (bossMusicCounter + 1) % 4260; // roughly 86 seconds; duration of the song
            if (gameOver) {
                printGameOver();
            }
            else {
                levelUpdate();
            }
            // ~60 fps, roughly
            std::this_thread::sleep_for(std::chrono::milliseconds(1000/60)); 
        }
    }
    system("killall afplay"); // end any remaining sounds or songs playing
    if (online) {
        delete C;
    }
    endwin();
}

void SpaceGame::frameUpdate() {
    clear();
    mvprintw(0,0, "Score: ");
    mvprintw(0,7, "%d", score);
    mvprintw(0, 12, "Level: ");
    mvprintw(0, 19, "%d", level);
    if (!isBossLevel) {
        mvprintw(0,116, "Remaining: ");
        mvprintw(0,127, "%d", 5*level - levelScore);
    }
    for (auto& projectile : projectiles) {
        mvprintw(projectile[1], projectile[0], "*");
    }
    for (auto& enemy : enemies) {
        mvprintw(enemy[1], enemy[0], "<");
    }
    for (auto& powerups : powerupProjectiles) {
        mvprintw(powerups[1], powerups[0], "@");
    }
    for (auto& enemy : deadEnemies) {
        if (enemy[2] > 0) {
            enemyExplosionAnimation(enemy[0],enemy[1]);
            enemy[2] -= 1;
        }
    }
    int powerupidx = 24;
    for (int i = 0; i < lives; i++) {
        mvprintw(0,22 + 3*i, "<3");
        powerupidx += 3;
    }
    if (playerHasPowerup) {
        mvprintw(0,powerupidx, "@");
    }
    if (powerupExists) {
        if (drawPowerupLife) {
            mvprintw(powerup[1], powerup[0], "@");
        }
    }
    if (lifeExists) {
        if (drawPowerupLife) {
            mvprintw(life[1], life[0], "<3");
        }
    }
    if (iframes > 0) {
        damageAnimation();
    }
    else {
        mvprintw(shipy, shipx, "-+>");
    }
    if (isBossLevel) {
        printBarrier();
        printBossHealthBar();
        for (auto& projectile : bossProjectiles) {
            mvprintw(projectile[1], projectile[0], "*");
        }
        if (!missile.empty()) {
            mvprintw(missile[1], missile[0], ">=>");
        }
        if (!bossMissile.empty()) {
            mvprintw(bossMissile[1], bossMissile[0], "<=<");
        }
        if (missileExists) {
            if (drawPowerupLife) {
                mvprintw(missileCollectible[1], missileCollectible[0], ">=>");
            }
        }
        if (playerHasMissile) {
            mvprintw(0,powerupidx + 2, ">=>");
        }
        if (bossDrawFrames > 0) {
            bossDamageAnimation();
        }
        else {
            printBoss();
        }
    }
    refresh();
}

void SpaceGame::keyRoute(int ch) {
    if (ch == 97 || ch == 100 || ch == 115 || ch == 119) {
        shipMoveUpdate(ch);
    }
    else if (ch == 32) {
        spawnProjectile();
    }
    else if (ch == 113 && gameOver) {
        if (isNewHighScore()) {
            highScoreUpdate();
        }
        printEndGame();
    }
    else if (ch == 101 && gameOver) {
        if (isNewHighScore()) {
            highScoreUpdate();
        }
        printScores();
        initialize();
        printStartOver();
    }
    else if (ch == 107 && playerHasPowerup) {
        usePowerup();
    }
    else if (ch == 109 && playerHasMissile) {
        spawnMissile();
    }
    else if (ch == 112) {
        pause();
    }
    else if (!gameStarted) {
        if (ch == 121) {
            establishDatabaseConnection();
            online = true;
            gameStarted = true;
        }
        if (ch == 110) {
            online = false;
            gameStarted = true;
        }
    }
}

void SpaceGame::projectileMoveUpdate() {
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        int currx = (*it)[0];
        if (!isBossLevel) {
            if (currx < WIDTH - 1) {
                (*it)[0] = currx + 2;
                ++it;
            }
            else {
                it = projectiles.erase(it);
            }
        }
        else {
            int mid23 = 2 * WIDTH / 3;
            if (currx < mid23 - 1) {
                (*it)[0] = currx + 2;
                ++it;
            }
            else {
                system("afplay Sounds/Barrier_Sound.mp3 >/dev/null 2>&1 &");
                it = projectiles.erase(it);
            }
        }
    }
}

void SpaceGame::shipMoveUpdate(int ch) {
    //up, w key
    if (ch == 119) {
        if (shipy != 1) {
           shipy--; 
        }
        else {
            shipy = HEIGHT - 1;
        }
    }
    //down, s key
    if (ch == 115) {
        if (shipy != HEIGHT - 1) {
            shipy++;
        }
        else {
            shipy = 1;
        }
    }
    //right, d key
    if (!isBossLevel) {
        if (ch == 100 && shipx < WIDTH - 3) {
        shipx += 2;
        }
    }
    else {
        int mid23 = 2 * WIDTH / 3;
        if (ch == 100 && shipx < mid23 - 3) {
        shipx += 2;
        }
    }
    //left, a key
    if (ch == 97 && shipx > 2) {
        shipx -= 2;
    }
}

void SpaceGame::updateEnemySpeed() {
    if (level == 5) {
        enemySpeed = 15;
    }
    if (level == 9) {
        enemySpeed = 12;
    }
    // if (level == 13) {    // too difficult
    //     enemySpeed = 10;
    // }
    // if (level == 17) {
    //     enemySpeed = 5;
    // }
    // if (level == 21) {
    //     enemySpeed = 3;
    // }
}

void SpaceGame::enemyMoveUpdate() {
    for (auto it = enemies.begin(); it != enemies.end();) {
        int currx = (*it)[0];
        if (currx > 2) {
            (*it)[0] = currx - 2;
            ++it;
        }
        else {
            system("afplay Sounds/Damage_v1.mp3 >/dev/null 2>&1 &");
            it = enemies.erase(it);
            lives--;
            iframes = 48;
            if (!lives) {
                // printGameOver();
                gameOver = true;
                return;
            }
            // frameUpdate();
        }
    }   
}

void SpaceGame::powerupMoveUpdate() {
for (auto it = powerupProjectiles.begin(); it != powerupProjectiles.end();) {
        int currx = (*it)[0];
        if (currx < WIDTH - 1) {
            (*it)[0] = currx + 2;
            ++it;
        }
        else {
            it = powerupProjectiles.erase(it);
        }
    }
    if (!powerupProjectiles.empty() && isBossLevel) {
        int x = powerupProjectiles[0][0];
        int mid23 = 2 * WIDTH / 3;
        if (abs(x - mid23) <= 1) {
            system("afplay Sounds/Barrier_Sound.mp3 >/dev/null 2>&1 &");
            powerupProjectiles.clear();
        }
    }
}

void SpaceGame::bossMoveUpdate(int direction) {
    if (direction == 1) {
        boss[1] -= 1;
    }
    if (direction == 0) {
        boss[1] += 1;
    }
}

void SpaceGame::bossMoveUpdateChance() {
    double chance = spawn_gen(rng) / (double) 2500;
    if (chance < 0.2) {
        int direction = spawn_gen(rng) % 2;
        int distance = spawn_gen(rng) % 5 + 3;
        if (direction == 1) {
            if (boss[1] - distance > 5) {
                boss[3] = 1;
                boss[4] = distance;
            }
        }
        else {
            if (boss[1] + distance < HEIGHT - 5) {
                boss[3] = 0;
                boss[4] = distance;
            }
        }
    }
}

void SpaceGame::missileMoveUpdate() {
    if (!missile.empty()) {
        if (missile[0] < WIDTH - 3) { // - 2 or - 3 ?
            missile[0]++;
        }
        else {
            missile.clear();
        }
    }
}

void SpaceGame::bossMissileMoveUpdate() {
    if (bossShotMissile) {
        int bossmx = bossMissile[0];
        int bossmy = bossMissile[1];
        if (bossmx > 0) {
            bossMissile[0] -= 1;
        }
        if (bossmx <= 0) {
            bossMissile.clear();
            bossShotMissile = false;
        } 
        if (shipx < bossmx && counter % 5 == 0) {
            if (shipy < bossmy) {
                if (bossmy - shipy > 6) {
                    bossMissile[1] -= 2;
                }
                else {
                    bossMissile[1] -= 1;
                }
                return;
            }
            if (shipy > bossmy) {
                if (shipy - bossmy > 6) {
                    bossMissile[1] += 2;
                }
                else {
                    bossMissile[1] += 1;
                }
                return;
            }
        }
    }
}

void SpaceGame::bossProjectileMoveUpdate() {
    for (auto it = bossProjectiles.begin(); it != bossProjectiles.end();) {
        int currx = (*it)[0];
        if (currx > 2) {
            (*it)[0] = currx - 2;
            ++it;
        }
        else {
            it = bossProjectiles.erase(it);
        }
    }   
}

void SpaceGame::spawnProjectile() {
    system("afplay Sounds/Shoot.mp3 >/dev/null 2>&1 &");
    int projx = shipx + 3;
    int projy = shipy;
    vector<int> projectile;
    projectile.push_back(projx);
    projectile.push_back(projy); 
    projectiles.push_back(projectile);
    shotsFired++;
}

void SpaceGame::spawnEnemy() {
    double enemySpawnChance = spawn_gen(rng) /((double) 2500);
    if (enemySpawnChance < threshold) {
        int enemyx = WIDTH - 1;
        int enemyy = enemy_y_gen(rng);
        if (isBossLevel) {
            if (abs(boss[1] - enemyy) <= 2) {
                enemyy += 4;
            }
            if (enemyy <= 3) {
                enemyy += 3;
            }
        }
        vector<int> enemy;
        enemy.push_back(enemyx);
        enemy.push_back(enemyy);
        enemies.push_back(enemy);
    }
}

void SpaceGame::spawnBoss() {
    boss.push_back(midw + 50);
    boss.push_back(midh);
    int health = 1 + (2*level) / 4;
    boss.push_back(health);
    boss.push_back(-1); // direction of movement; 1 for up, 0 for down, -1 default 
    boss.push_back(0); // distance to move
}

void SpaceGame::spawnPowerup() {
    double chance = spawn_gen(rng) / (double) 2500;
    double thresh = (24 + level) / (double) 100;
        if (isBossLevel) {
        thresh = 0.15;
    }
    if (chance < thresh) {
        int powerupx = 0;
        int mid23 = 2 * WIDTH / 3;
        if (isBossLevel) {
            powerupx = powerupLifexBossLevel_gen(rng);
        }
        else {
            powerupx = powerupLifex_gen(rng);
        }
        int powerupy = powerupLifey_gen(rng);
        powerup.push_back(powerupx);
        powerup.push_back(powerupy);
        if (!isBossLevel) {
            spawnedPowerup = true;  // on boss levels, multiple lives and powerups may spawn
        }
        powerupExists = true;
    }
}

void SpaceGame::powerupCollected() {
    if (shipx == powerup[0] || shipx + 1 == powerup[0] || shipx + 2 == powerup[0]) {
        if (shipy == powerup[1]) {
            system("afplay Sounds/Powerup_Pickup.mp3 >/dev/null 2>&1 &");
            playerHasPowerup = true;
            powerupExists = false;
            powerup.clear();
        }
    }
}

void SpaceGame::despawnPowerup() {
    powerup.clear();
    powerupExists = false;
}

void SpaceGame::usePowerup() {
    for (int i = 1; i < HEIGHT; i++) {
        vector<int> powerupProjectile;
        powerupProjectile.push_back(shipx);
        powerupProjectile.push_back(i);
        powerupProjectiles.push_back(powerupProjectile);
    }
    system("afplay Sounds/Powerup_Sound.mp3 >/dev/null 2>&1 &");
    playerHasPowerup = false;
    powerupsUsed++;
}

void SpaceGame::spawnLife() {
    double chance = spawn_gen(rng) / (double) 2500;
    double thresh = (24 + level) / (double) 100;
    if (isBossLevel) {
        thresh = 0.15;
    }
    if (chance < thresh) {
        int lifex = 0;
        int mid23 = 2 * WIDTH / 3;
        if (isBossLevel) {
            lifex = powerupLifexBossLevel_gen(rng);
        }
        else {
            lifex = powerupLifex_gen(rng);
        }
        int lifey = powerupLifey_gen(rng);
        life.push_back(lifex);
        life.push_back(lifey);
        if (!isBossLevel) {
            spawnedLife = true;
        }
        lifeExists = true;
    }
}

void SpaceGame::lifeCollected() { // fix this; i.e. if shipx == life[0] + 1 || ...
    if (shipx == life[0] || shipx + 1 == life[0] || shipx + 2 == life[0] || shipx == life[0] + 1) {
        if (shipy == life[1]) {
            system("afplay Sounds/Life_Sound_Effect.mp3 >/dev/null 2>&1 &");
            lives++;
            lifeExists = false;
            livesUsed++;
            life.clear();
        }
    }
}

void SpaceGame::despawnLife() {
    life.clear();
    lifeExists = false;
}

void SpaceGame::spawnMissileCollectible() {
    double chance = spawn_gen(rng) / (double) 2500;
    int mid23 = 2 * WIDTH / 3;
    if (chance < 0.5) {
        int missilex = powerupLifex_gen(rng) % (mid23 - 2);
        int missiley = powerupLifey_gen(rng);
        if (missiley < 4) {
            missiley += 2;
        }
        missileCollectible.push_back(missilex);
        missileCollectible.push_back(missiley);
        missileExists = true;
    }
}

void SpaceGame::missileCollected() {
    if (abs(shipx - missileCollectible[0]) <= 2 && shipy == missileCollectible[1]) {   
        playerHasMissile = true;
        missileCollectible.clear();
        missileExists = false;
        system("afplay Sounds/Missile_Pickup.mp3 >/dev/null 2>&1 &");
    }
}

void SpaceGame::despawnMissileCollectible() {
    missileCollectible.clear();
    missileExists = false;
}

void SpaceGame::spawnMissile() {
    int missilex = shipx + 3;
    int missiley = shipy;
    missile.push_back(missilex);
    missile.push_back(missiley);
    shotsFired++;
    playerHasMissile = false;
    system("afplay Sounds/Missile_Launch_v1.mp3 >/dev/null 2>&1 &");
}

void SpaceGame::spawnBossMissile() {
    double chance = spawn_gen(rng) / (double) 2500;
    double thresh = 0.3 + 0.1 * ((level / (double) 4) - 1.0);
    if (chance < thresh) {
        bossMissile.push_back(boss[0] - 7);
        bossMissile.push_back(boss[1]);
        bossShotMissile = true;
        system("afplay Sounds/Boss_Missile.mp3 >/dev/null 2>&1 &");
    }
}

void SpaceGame::spawnBossProjectiles() {
    double chance = spawn_gen(rng) / (double) 2500;
    double thresh = 0.2 + 0.05 * ((level / (double) 4) - 1.0);
    if (chance < thresh) {
        system("afplay Sounds/Boss_Projectile_v4.mp3 >/dev/null 2>&1 &");
        vector<int> proj1; proj1.push_back(boss[0] - 7); proj1.push_back(boss[1]);
        vector<int> proj2; proj2.push_back(boss[0] - 2); proj2.push_back(boss[1] + 1);
        vector<int> proj3; proj3.push_back(boss[0] - 2); proj3.push_back(boss[1] - 1);
        vector<int> proj4; proj4.push_back(boss[0] + 3); proj4.push_back(boss[1] + 2);
        vector<int> proj5; proj5.push_back(boss[0] + 3); proj5.push_back(boss[1] - 2);
        bossProjectiles.push_back(proj1); bossProjectiles.push_back(proj2);
        bossProjectiles.push_back(proj3); bossProjectiles.push_back(proj4);
        bossProjectiles.push_back(proj5);
    }
}

void SpaceGame::enemyProjectileCollisionUpdate() {
    for (auto itp = projectiles.begin(); itp != projectiles.end();) {
        bool projerased = false;
        auto projectile = (*itp);
        for (auto ite = enemies.begin(); ite != enemies.end();) {
            auto enemy = (*ite);
            if (abs(projectile[0] - enemy[0]) <= 1 && projectile[1] == enemy[1]) {
                itp = projectiles.erase(itp);
                projerased = true;
                enemy.push_back(5);
                deadEnemies.push_back(enemy);
                ite = enemies.erase(ite);
                system("afplay Sounds/Explosion.mp3 >/dev/null 2>&1 &");
                numHits++;
                score++;
                enemiesKilled++;
                if (!isBossLevel) {
                    levelScore++;
                }
                // frameUpdate();
                // levelUpdate();
                break;
            }
            else {
                ite++;
            }
        }
        if (projerased) {
            continue;
        }
        else {
            itp++;
        }
    }
}

void SpaceGame::bossMissileCollisionUpdate() {
    if (!missile.empty()) {
        if (missile[1] == boss[1] && (boss[0] - missile[0]) <= 6) {
            system("afplay Sounds/Boss_Damage.mp3 >/dev/null 2>&1 &");
            missile.clear();
            boss[2] -= 1;
            numHits++;
            bossDrawFrames = 36;
            // frameUpdate();
            if (boss[2] == 0) {
                bossExplosionAnimation();
                // levelUpdate();
            }
            return;
        }
        if (abs(missile[1] - boss[1]) == 1 && (boss[0] - missile[0]) <= 1) {
            system("afplay Sounds/Boss_Damage.mp3 >/dev/null 2>&1 &");
            missile.clear();
            boss[2] -= 1;
            numHits++;
            bossDrawFrames = 36;
            // frameUpdate();
            if (boss[2] == 0) {
                bossExplosionAnimation();
                // levelUpdate();
            }
            return;
        }
        if (abs(missile[1] - boss[1]) == 2 && (boss[0] - missile[0]) <= 1) { //?
            system("afplay Sounds/Boss_Damage.mp3 >/dev/null 2>&1 &");
            missile.clear();
            boss[2] -= 1;
            numHits++;
            bossDrawFrames = 36;
            // frameUpdate();
            if (boss[2] == 0) {
                bossExplosionAnimation();
                // levelUpdate();
            }
            return;
        }
    }
}

void SpaceGame::enemyMissileCollisionUpdate() {
    if (!missile.empty()) {
        for (auto it = enemies.begin(); it != enemies.end();) {
            auto enemy = (*it);
            if (enemy[0] - missile[0] <= 2 && missile[1] == enemy[1]) {
                system("afplay Sounds/Explosion.mp3 >/dev/null 2>&1 &");
                enemy.push_back(5);
                deadEnemies.push_back(enemy);
                it = enemies.erase(it);
                score++;
                enemiesKilled++;
                // frameUpdate();
            }
            else {
                it++;
            }
        }
    }
}

void SpaceGame::shipBossMissileCollisionUpdate() {
    if (bossShotMissile) {
        if (abs(bossMissile[0] - shipx) <= 2 && bossMissile[1] == shipy) {
            if (!iframes) {
                system("afplay Sounds/Damage_v1.mp3 >/dev/null 2>&1 &");
                lives--;
                iframes = 48;
            }
            bossShotMissile = false;
            bossMissile.clear();
            // frameUpdate();
            if (!lives) {
                // printGameOver();
                gameOver = true;
                return;
            }
        }
    }
}

void SpaceGame::shipBossProjectileCollisionUpdate() {
    for (auto it = bossProjectiles.begin(); it != bossProjectiles.end();) {
        auto projectile = (*it);
        if ((shipx == projectile[0] || shipx + 1 == projectile[0] || shipx + 2 == projectile[0])
        && shipy == projectile[1]) {
            it = bossProjectiles.erase(it);
            if (!iframes) {
                system("afplay Sounds/Damage_v1.mp3 >/dev/null 2>&1 &");
                lives--;
                iframes = 48;
            }
            // frameUpdate();
            if (!lives) {
                // printGameOver();
                gameOver = true;
                return;
            }
        }
        else {
            it++;
        }
    }
}

void SpaceGame::shipEnemyCollisionUpdate() {
    for (auto it = enemies.begin(); it != enemies.end();) {
        auto enemy = (*it);
        if ((shipx == enemy[0] || shipx + 1 == enemy[0] || shipx + 2 == enemy[0])
        && shipy == enemy[1]) {
            enemy.push_back(5);
            deadEnemies.push_back(enemy);
            it = enemies.erase(it);
            if (!iframes) {
                system("afplay Sounds/Damage_v1.mp3 >/dev/null 2>&1 &");
                lives--;
                iframes = 48;
            }
            // frameUpdate();
            if (!lives) {
                // printGameOver();
                gameOver = true;
                return;
            }
        }
        else {
            it++;
        }
    }
}

void SpaceGame::enemyPowerupCollisionUpdate() {
    if (powerupProjectiles.size() > 0) {
        int xcheck = powerupProjectiles[0][0];
        for (auto it = enemies.begin(); it != enemies.end();) {
            auto enemy = (*it);
            if (abs(xcheck - enemy[0]) <= 2) {
                enemy.push_back(5);
                deadEnemies.push_back(enemy);
                it = enemies.erase(it);
                score += 2;  // extra score given when powerups kill enemies
                enemiesKilled++;
                if (!isBossLevel) {
                    levelScore++;
                }
                system("afplay Sounds/Explosion.mp3 >/dev/null 2>&1 &");
                // frameUpdate();
                // levelUpdate();
            }
            else {
                it++;
            }
        }
        if (isBossLevel) {
            for (auto it = bossProjectiles.begin(); it != bossProjectiles.end();) {
                auto projectile = (*it);
                if (abs(xcheck - projectile[0]) <= 2) {
                    it = bossProjectiles.erase(it);
                    // frameUpdate();
                }
                else {
                    it++;
                }
            }
            if (bossShotMissile) {
                if (abs(xcheck - bossMissile[0]) <= 2) {
                    bossMissile.clear();
                    // frameUpdate();
                    bossShotMissile = false;
                }
            }
        }
    }
}

void SpaceGame::levelUpdate() {
    if (!isBossLevel) {
        if (levelScore == 5 * level) {
            level++;
            printNewLevel();
        }
    }
    else {
        if (boss[2] == 0) {
            level++;
            score += 10 * (level / 2);
            printNewLevel();
        }
    }
}

void SpaceGame::damageAnimation() {
    if (iframes % 8 == 0) { // iframes total starts at 48; 
        shipDraw = !shipDraw;
    }
    if (shipDraw) {
        mvprintw(shipy, shipx, "-+>");
    }
    else {
        mvprintw(shipy, shipx, "   ");
    }
    iframes--;
}

void SpaceGame::bossDamageAnimation() {
    if (bossDrawFrames % 6 == 0) { // bossDrawFrames starts at 36;
        bossDraw = !bossDraw;
    }
    if (bossDraw) {
        printBoss();
    }
    else {
        unprintBoss();
    }
    bossDrawFrames--;
}

void SpaceGame::bossExplosionAnimation() {
    system("killall afplay");
    printBossHealthBar();
    refresh();
    mvprintw(boss[1], boss[0] - 6,"<=&=&===&===");
    mvprintw(boss[1] - 1, boss[0] - 1, "<=&==&=&=");
    mvprintw(boss[1] + 1, boss[0] - 1, "<==&===");
    mvprintw(boss[1] - 2, boss[0] + 4, "&===");
    mvprintw(boss[1] + 2, boss[0] + 4, "<=&=");
    refresh();
    system("afplay Sounds/Explosion.mp3 >/dev/null 2>&1 &");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    mvprintw(boss[1], boss[0] - 6,"&&&#&&&&#&&&");
    mvprintw(boss[1] - 1, boss[0] - 1, "&&&#&&&&&");
    mvprintw(boss[1] + 1, boss[0] - 1, "&&#&&&&");
    mvprintw(boss[1] - 2, boss[0] + 4, "#&&&");
    mvprintw(boss[1] + 2, boss[0] + 4, "&&&&");
    refresh();
    system("afplay Sounds/Explosion.mp3 >/dev/null 2>&1 &");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    //mvprintw(boss[1] - 1, boss[0] )
    mvprintw(boss[1], boss[0] - 8,"&   # &   &  #   &");
    mvprintw(boss[1] - 1, boss[0] - 1, "  &  #   &");
    mvprintw(boss[1] + 1, boss[0] - 2, "&  #  #  ");
    mvprintw(boss[1] - 2, boss[0] + 3, "&  & ");
    mvprintw(boss[1] + 2, boss[0] + 4, "# & ");
    mvprintw(boss[1] + 3, boss[0] + 3, " & ");
    mvprintw(boss[1] - 3, boss[0], "#    &");
    refresh();
    system("afplay Sounds/Explosion.mp3 >/dev/null 2>&1 &");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    mvprintw(boss[1], boss[0] - 8,"&         &      &");
    mvprintw(boss[1] - 1, boss[0] - 1, "  &  #    ");
    mvprintw(boss[1] + 1, boss[0] - 2, "      #  ");
    mvprintw(boss[1] - 2, boss[0] + 3, "&    ");
    mvprintw(boss[1] + 2, boss[0] + 4, "    ");
    mvprintw(boss[1] + 3, boss[0] + 3, " & ");
    mvprintw(boss[1] - 3, boss[0], "     &");
    refresh();
    system("afplay Sounds/Explosion.mp3 >/dev/null 2>&1 &");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    mvprintw(boss[1], boss[0] - 8,"                  ");
    mvprintw(boss[1] - 1, boss[0] - 1, "          ");
    mvprintw(boss[1] + 1, boss[0] - 2, "         ");
    mvprintw(boss[1] - 2, boss[0] + 3, "     ");
    mvprintw(boss[1] + 2, boss[0] + 4, "    ");
    mvprintw(boss[1] + 3, boss[0] + 3, "   ");
    mvprintw(boss[1] - 3, boss[0], "      ");
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    system("afplay Sounds/Boss_Defeated_v1.mp3 >/dev/null 2>&1 &");
    mvprintw(HEIGHT / 2, WIDTH / 2 - 12, "Boss defeated! +   Score");
    mvprintw(HEIGHT / 2, WIDTH / 2 + 4, "%d", 10*(level/2));
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    mvprintw(HEIGHT / 2 - 2, WIDTH / 2 - 12, "                        ");
    refresh();
    bossesKilled++;
}

void SpaceGame::enemyExplosionAnimation(int x, int y) {
    mvprintw(y + 1, x, "&");
    mvprintw(y - 1, x, "&");
    mvprintw(y, x - 1, "&");
    mvprintw(y, x + 1, "&");
}

void SpaceGame::clearDeadEnemies() {
    for (auto it = deadEnemies.begin(); it != deadEnemies.end();) {
        auto enemy = *it;
        if (enemy[2] == 0) {
            it = deadEnemies.erase(it);
        } 
        else {
            ++it;
        }
    }
}

void SpaceGame::initialize() {
    shipx = WIDTH / 2;
    shipy = HEIGHT / 2;
    score = 0;
    shotsFired = 0;
    enemySpeed = 20;
    keepPlaying = true;
    gameOver = false;
    level = 1;
    levelScore = 0;
    threshold = 0.004;
    counter = 1;
    lives = 3;
    enemies.clear();
    projectiles.clear();
    spawnedPowerup = false;
    powerupExists = false;
    spawnedLife = false;
    lifeExists = false;
    drawPowerupLife = false;
    isBossLevel = false;
    iframes = 0;
    bossDraw = true;
    shipDraw = true;
    boss.clear();
    missile.clear();
    missileCollectible.clear();
    bossProjectiles.clear();
    bossMissile.clear();
    deadEnemies.clear();
    powerupProjectiles.clear();
    life.clear();
    powerup.clear();
    updateEnemySpeed();
    bossDrawFrames = 0;
    playerHasPowerup = false;
    playerHasMissile = false;
    livesUsed = 3;
    powerupsUsed = 0;
    enemiesKilled = 0;
    bossesKilled = 0;
    accuracy = 0;
    nodelay(stdscr, TRUE);
}

void SpaceGame::printBoss() {
    mvprintw(boss[1], boss[0] - 6,"<===========");
    mvprintw(boss[1] - 1, boss[0] - 1, "<=======");
    mvprintw(boss[1] + 1, boss[0] - 1, "<=======");
    mvprintw(boss[1] - 2, boss[0] + 4, "<===");
    mvprintw(boss[1] + 2, boss[0] + 4, "<===");
}

void SpaceGame::unprintBoss() {
    mvprintw(boss[1], boss[0] - 6,"            ");
    mvprintw(boss[1] - 1, boss[0] - 1, "        ");
    mvprintw(boss[1] + 1, boss[0] - 1, "        ");
    mvprintw(boss[1] - 2, boss[0] + 4, "    ");
    mvprintw(boss[1] + 2, boss[0] + 4, "    ");
}

void SpaceGame::printBossHealthBar() {
    int midw = WIDTH / 2;
    int health = boss[2];
    int total = 1 + 2 * (level / 4);
    mvprintw(2, midw - total -1, "[");
    for (int i = 0; i < total; i++) {
        if (health > 0) {
            mvprintw(2, midw - total + 2*i, "==");
            health--;
        }
    }
    mvprintw(2, midw + total, "]");
    mvprintw(1, midw - 2, "BOSS");
}

void SpaceGame::printBarrier() {
    int mid23 = 2* WIDTH / 3;
    for (int i = 0; i < HEIGHT; i++) {
        mvprintw(i, mid23, "|");
    }
}

void SpaceGame::pause() {
    system("killall afplay");
    bossMusicCounter = 0; 
    clear();
    frameUpdate();
    mvprintw(HEIGHT / 2, WIDTH / 2 - 3, "PAUSED");
    refresh();
    int ch;
    while(1) {
        if ((ch = getch()) == 112) {
            break;
        }
        // slows down execution of the loop to reduce resourc use intensity
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
    }
    clear();
    frameUpdate();
}

void SpaceGame::printStartGame() {
    printLogo();
    system("afplay Sounds/Main_Theme_v2.mp3 >/dev/null 2>&1 &");
    mvprintw(midh + 6, midw - 11, "Press any key to begin.");
    nodelay(stdscr, FALSE);
    getch();
}

void SpaceGame::printLogo() {
    /**
     *     * * *      * * *       * * *        * * *    * * * *     * * *       * * *      * *   * *    * * * *  
     *    *         *       *   *       *    *          *         *           *       *   *    *    *   *
     *     * * *    * * * *     * * * * *   *           * * * *   *    * *    * * * * *   *    *    *   * * * *
     *          *   *           *       *    *          *         *       *   *       *   *         *   *  
     *     * * *    *           *       *      * * *    * * * *    * * * *    *       *   *         *   * * * *  
    */
    mvprintw(6, midw - 50, "* * *      * * *       * * *        * * *    * * * *     * * *       * * *      * *   * *    * * * * ");
    mvprintw(7, midw - 51, "*         *       *   *       *    *          *         *           *       *   *    *    *   * ");
    mvprintw(8, midw - 50, "* * *    * * * *     * * * * *   *           * * * *   *    * *    * * * * *   *    *    *   * * * *");
    mvprintw(9, midw - 45, "*   *           *       *    *          *         *       *   *       *   *         *   *");
    mvprintw(10, midw - 50, "* * *    *           *       *      * * *    * * * *    * * * *    *       *   *         *   * * * *");
}

void SpaceGame::printRules() {
    mvprintw(midh - 2, midw - 26, "Stop the enemy from reaching your side! Use the W, A, S, D");
    mvprintw(midh - 1, midw - 26, "keys to move, SPACE to shoot, K for powerups, and P to");
    mvprintw(midh, midw - 26, "pause the game. Kill enemies to progress to the next level.");
    mvprintw(midh + 1, midw - 26, "Look for powerups '@' and extra lives '<3'. You can only");
    mvprintw(midh + 2, midw - 26, "have one powerup at a time; use it wisely. Good luck! ");
    mvprintw(midh + 6, midw - 13, "Press any key to continue.");
    refresh();
    getch();
    clear();
}

void SpaceGame::printOnlineMode() {
    clear();
    printLogo();
    mvprintw(midh - 2, midw - 10, "Play in online mode?");
    mvprintw(midh + 0, midw - 30, "In online mode, your scores will be recorded in the online");
    mvprintw(midh + 1, midw - 30, "database. In offline mode, your scores will be deleted after");
    mvprintw(midh + 2, midw - 30, "the current game session is over.");
    mvprintw(midh + 4, midw - 6, "Press Y or N.");
    refresh();
    int ch;
    while (1) {
        ch = getch();
        if (ch == 121) { // Y
            keyRoute(ch);
            break;
        }
        else if (ch == 110) { // N
            keyRoute(ch);
            clear();
            printLogo();
            mvprintw(midh, midw - 16, "Okay. Continuing in offline mode.");
            refresh();
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
    }
    clear();
    system("killall afplay");
}

bool SpaceGame::isNewHighScore() {
    if (online) {
        pqxx::work W(*C);
        pqxx::result resultCount = W.exec("SELECT COUNT(*) FROM spacegame");
        int scoreCount = atoi(resultCount[0][0].c_str());
        if (scoreCount < 10) { // we only care about the 10 highest scores
            return true;
        }
        else {
            pqxx::result r = W.exec("SELECT score FROM spacegame ORDER BY score DESC LIMIT 10");
            int minHighScore = atoi(r[9][0].c_str());
            if (score > minHighScore) {
                return true;
            }
        }
    }
    else {
        if (highScores.size() < 3) { // in offline mode, we only track the 3 highest scores
            return true;
        }
        for (auto hscore : highScores) {
            if (score > hscore) {  // > instead of >=; gives precedence to existing scores; have to "beat" it
                return true;
            }
        }
    }
    return false;
}

void SpaceGame::highScoreUpdate() {
    clear();
    system("killall afplay");
    system("afplay Sounds/New_High_Score.mp3 >/dev/null 2>&1 &");
    printLogo();
    mvprintw(midh - 2, midw - 9, "New High Score!");
    mvprintw(midh - 2, midw + 7, "%d", score);
    mvprintw(midh, midw - 14, "Type in a 3 letter name: ");
    refresh();
    nodelay(stdscr, FALSE);
    int charCount = 0;
    string name = "";
    while (charCount < 3) {
        noecho();
        char ch = (char) getch();
        ch = toupper(ch);
        name.push_back(ch);
        auto outch = &ch;
        mvprintw(midh, midw + 11 + charCount, outch);
        mvprintw(midh, midw + 12 + charCount, "    ");
        refresh();
        charCount++;
    }
    if (online) {
        pqxx::work W(*C);
        string injection = insertString(name);
        W.exec(injection);
        W.commit();
    }
    else {
        int idx = 0;
        for (auto it = highScores.begin(); it != highScores.end();) {
            if (score > (*it)) {
                highScores.insert(it, score);
                highScores.pop_back();
                break;
            }
            ++it;
            idx++;
        }
        names.insert(names.begin() + idx, name); // this could possibly crash if idx = 3 ... test
        if (names.size() > 3) {
            names.pop_back();
        }
    }
    mvprintw(midh + 2, midw - 10, "New High Score added!");
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    clear();
}

void SpaceGame::enterInitials() {
    printLogo();
    mvprintw(midh - 2, midw - 29, "Enter your initials below to keep a record of your score.");
    mvprintw(midh, midw - 14, "Type in a 3 letter name: ");
    refresh();
    nodelay(stdscr, FALSE);
    int charCount = 0;
    string name = "";
    while (charCount < 3) {
        noecho();
        char ch = (char) getch();
        ch = toupper(ch);
        name.push_back(ch);
        auto outch = &ch;
        mvprintw(midh, midw + 11 + charCount, outch);
        mvprintw(midh, midw + 12 + charCount, "    ");
        refresh();
        charCount++;
    }
    string injection = insertString(name);
    pqxx::work W(*C);
    W.exec(injection);
    W.commit();
    mvprintw(midh + 2, midw - 8, "New Score added!");
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    clear();
}

void SpaceGame::printGameOver() {
    system("killall afplay");
    system("afplay Sounds/Game_Over.mp3 >/dev/null 2>&1 &");
    // gameOver = true;
    mvprintw(midh - 2, midw - 6, "GAME OVER");
    mvprintw(midh, midw - 6, "Score: ");
    mvprintw(midh, midw + 1, "%d", score);
    mvprintw(midh + 1, midw - 6, "Level: ");
    mvprintw(midh + 1, midw + 1, "%d", level);
    mvprintw(midh + 2, midw - 6, "Accuracy: ");
    if (shotsFired > 0) {
        accuracy = (int) (100.0 * (numHits / ((double) shotsFired)));
    }
    mvprintw(midh + 2, midw + 4, "%d", accuracy);
    if (accuracy == 100) {
        mvprintw(midh + 2, midw + 7, "%%");
    }
    else {
        mvprintw(midh + 2, midw + 6, "%%");
    }
    mvprintw(midh + 4, midw - 20, "Press 'e' to play again, or 'q' to quit.");
    refresh();
    int ch;
    while (1) {
        ch = getch();
        if (ch == 113 || ch == 101) {
            keyRoute(ch);
            break;
        }
    }
}

void SpaceGame::printNewLevel() {
    if (score > 0) {
        system("afplay Sounds/Level_Up.mp3 >/dev/null 2>&1 &");
        if (level < 10) {
            mvprintw(midh, midw - 18, "Level complete! Moving to level  ...");
        }
        else {
            mvprintw(midh, midw - 18, "Level complete! Moving to level   ...");
        }
        mvprintw(midh, midw +14, "%d", level);
        refresh();
        projectiles.clear();
        enemies.clear();
        deadEnemies.clear();
        shipx = midw;
        shipy = midh;
        levelScore = 0;
        threshold = 0.004 + 0.0006 * level; // 4 + (3/5)x
        spawnedPowerup = false;
        powerupExists = false;
        spawnedLife = false;
        lifeExists = false;
        isBossLevel = false;
        missileExists = false;
        bossShotMissile = false;
        playerHasMissile = false;
        bossDraw = true;
        shipDraw = true;
        iframes = 0;
        bossDrawFrames = 0;
        boss.clear();
        missile.clear();
        missileCollectible.clear();
        bossProjectiles.clear();
        bossMissile.clear();
        powerupProjectiles.clear();
        life.clear();
        powerup.clear();
        updateEnemySpeed();
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        clear();
    }
    if (level % 4 == 0) {
        system("afplay Sounds/Impending_Battle.mp3 >/dev/null 2>&1 &");
        isBossLevel = true;
        iframes = 48;
        bossMusicCounter = 0;
        spawnBoss();
        if (level == 4) {
            mvprintw(midh - 3, midw - 36, "This is a boss level! The boss shoots projectiles and homing missiles at");
            mvprintw(midh - 2, midw - 36, "you. You must dodge them or destroy them with powerups. The boss generates");
            mvprintw(midh - 1, midw - 36, "a barrier that blocks your projectiles and powerups. However, missiles");
            mvprintw(midh + 0, midw - 36, "capable of passing the barrier spawn in the area. Get them and press M");
            mvprintw(midh + 1, midw - 36, "to fire them at the boss. Look out for regular enemies as well. Good luck!");
            mvprintw(midh + 6, midw - 13, "Press any key when ready.");
            refresh();
            nodelay(stdscr, FALSE);
            getch();
            system("killall afplay");
            clear();
        }
        else {
            system("afplay Sounds/Impending_Battle.mp3 >/dev/null 2>&1 &");
            mvprintw(midh, midw - 16, "This is a boss level! Good luck!");
            refresh();
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            system("killall afplay");
            clear();
        }
    } 
    else {
        if (level < 10) {
            mvprintw(midh - 2, midw - 3, "Level ");
            mvprintw(midh - 2, midw + 3, "%d", level);
        }
        else {
            mvprintw(midh - 2, midw - 4, "Level ");
            mvprintw(midh - 2, midw + 2, "%d", level);
        }
        mvprintw(midh, midw - 13, "Kill ");
        mvprintw(midh, midw - 8, "%d", 5*level);
        if (level == 1) {
            mvprintw(midh, midw - 6, "enemies to advance.");
        }
        else {
            mvprintw(midh, midw - 5, "enemies to advance.");
        }
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    nodelay(stdscr, TRUE);
}

void SpaceGame::printScores() {
    clear();
    printLogo();
    if (online) {
        mvprintw(midh - 8, midw - 5, "High Scores:");
        int idx = 0;
        pqxx::work W(*C);
        pqxx::result scores = W.exec("SELECT name, score FROM spacegame ORDER BY score DESC LIMIT 10");
        for (auto const & row : scores) {
            auto rowName = row[0].c_str();
            auto rowScore = row[1].c_str();
            mvprintw(midh - 6 + idx, midw - 7, rowName);
            // mvprintw(midh - 6 + idx, midw, ":");
            mvprintw(midh - 6 + idx, midw + 3, rowScore);
            idx++;
        }
        mvprintw(midh - 4 + idx, midw - 11, "Press any key to exit.");
    }
    else {
        mvprintw(midh - 2, midw - 6, "High Scores:");
        int idx = 0;
        while (idx < names.size()) {
            auto name = names[idx].c_str();
            mvprintw(midh + idx, midw - 7, name);
            // mvprintw(midh + idx, midw, ":");
            mvprintw(midh + idx, midw + 3, "%d", highScores[idx]);
            idx++;
        }
        mvprintw(midh + 2 + idx, midw - 11, "Press any key to exit.");
    }
    refresh();
    nodelay(stdscr, FALSE);
    getch();
}

void SpaceGame::printStartOver() {
    clear();
    mvprintw(midh, midw - 8, "Starting new game...");
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    clear();
    printLogo();
    mvprintw(midh, midw - 10, "Press any key to begin!");
    refresh();
    noecho();
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

void SpaceGame::printEndGame() {
    clear();
    int idx = 0;
    printLogo();
    if (online) {
        mvprintw(midh - 6, midw - 25, "Okay, thanks for playing! Here are the top scores:");
        int idx = 0;
        pqxx::work W(*C);
        pqxx::result scores = W.exec("SELECT name, score FROM spacegame ORDER BY score DESC LIMIT 10");
        for (auto const & row : scores) {
            auto rowName = row[0].c_str();
            auto rowScore = row[1].c_str();
            mvprintw(midh - 4 + idx, midw - 7, rowName);
            // mvprintw(midh - 4 + idx, midw, ":");
            mvprintw(midh - 4 + idx, midw + 3, rowScore);
            idx++;
        }
        mvprintw(midh - 2 + idx, midw - 11, "Press any key to quit.");
    }
    else {
        mvprintw(midh - 2, midw - 25, "Okay, thanks for playing! Here are the top scores:");
        int idx = 0;
        while (idx < names.size()) {
            auto name = names[idx].c_str();
            mvprintw(midh + idx, midw - 7, name);
            // mvprintw(midh + idx, midw - 1, ":");
            mvprintw(midh + idx, midw + 3, "%d", highScores[idx]);
            idx++;
        }
        mvprintw(midh + 2 + idx, midw - 11, "Press any key to quit.");
    }
    refresh();
    nodelay(stdscr, FALSE);
    getch();
    keepPlaying = false;
}

void SpaceGame::establishDatabaseConnection() {
    clear();
    try {
        mvprintw(midh, midw - 17, "Establishing server connection...");
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        C = new pqxx::connection(
            "dbname = " + config.DB_NAME +
            "host = localhost "
            "port = 5433 "
            "user = " + config.DB_USERNAME +
            "password = " + config.DB_PASSWORD
        );
        clear();
        mvprintw(midh, midw - 18, "Connection established successfully!");
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
    catch (const std::exception& e) {
        clear();
        mvprintw(midh, midw - 21, "Connection failed; playing in offline mode.");
        refresh();
        online = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
    clear();
}

string SpaceGame::insertString(string name) {
    transform(name.begin(), name.end(), name.begin(), ::toupper);
    double doubleAccuracy = accuracy / (double) 100;
    string stringScore = std::to_string(score);
    string stringAccuracy = std::to_string(doubleAccuracy);
    string stringLevel = std::to_string(level);
    string enemies_killed = std::to_string(enemiesKilled);
    string bosses_killed = std::to_string(bossesKilled);
    string shots_fired = std::to_string(shotsFired);
    string powerups_used = std::to_string(powerupsUsed);
    string lives_used = std::to_string(livesUsed);
    string SQL = "INSERT INTO spacegame (name, score, accuracy, "
    "level, enemies_killed, bosses_killed, shots_fired, powerups_used, lives_used) "
    "VALUES ('" + name + "', " + stringScore + ", " + stringAccuracy + ", " +
    stringLevel + ", " + enemies_killed + ", " + bosses_killed + ", " +
    shots_fired + ", " + powerups_used + ", " + lives_used + ");";
    return SQL;
}

int SpaceGame::abs(int a) {
    if (a < 0) {
        return -a;
    }
    return a;
}

void SpaceGame::debug() {
    mvprintw(midh, midw, "Got here");
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    clear();
    mvprintw(midh, midw, "Next one...");
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    clear();
}
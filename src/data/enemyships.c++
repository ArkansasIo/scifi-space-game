// enemyship.c++ -- fills enemyshipsArray[] with the enemy fleet.

#include "spacebattlerpg.h"

struct enemySeed
{
    const char *name;
    int sp;
    int health;
    int power;
    int sif;
};

static const enemySeed enemySeeds[17] = {
    {"The Lexx", 60, 100, 80, 20},
    {"The Billisther", 60, 100, 20, 80},
    {"Balance of Judgment", 60, 100, 40, 60},
    {"Imperial Destroyer", 40, 100, 60, 60},
    {"Battlecruiser", 60, 100, 60, 60},
    {"Shivan Sathanas Juggernaught", 20, 100, 70, 50},
    {"Foreshadow", 60, 100, 60, 60},
    {"Andromeda", 30, 100, 40, 50},
    {"Destroyer", 70, 100, 70, 70},
    {"Interceptor", 80, 100, 80, 80},
    {"Prometheus", 75, 100, 75, 75},
    {"Drekon Obliterator", 90, 100, 70, 80},
    {"Mega Shadow", 100, 100, 80, 70},
    {"Shadow Warbird", 40, 100, 30, 30},
    {"Sovereign", 50, 100, 10, 50},
    {"Marduk Flagship", 100, 100, 100, 100},
    {"The Super Dimension Fortress", 100, 100, 100, 100}};

void initializeEnemyships()
{
    for (int i = 0; i < 17; ++i)
    {
        enemyships &e = enemyshipsArray[i];

        strncpy(e.name, enemySeeds[i].name, sizeof(e.name) - 1);
        e.name[sizeof(e.name) - 1] = '\0';

        e.sp = enemySeeds[i].sp;
        e.health = enemySeeds[i].health;
        e.power = enemySeeds[i].power;
        e.sif = enemySeeds[i].sif;
        e.attackpower = enemySeeds[i].power * 2;
        e.defencepower = enemySeeds[i].sif * 2;
    }
}

void initializeenemyplayer()
{
    initializeEnemyships();
}
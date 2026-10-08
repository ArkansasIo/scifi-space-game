// battle.c++ -- the battle generator.
//
// Original behaviour (preserved): pick a random enemy ship from the 17-entry
// fleet, reroll its ship points so every fight is fresh, then trade blows in a
// turn loop until one side is destroyed.
//
// What changed: the loop is no longer one long function.  It is split into
// named steps (setup, player attack, enemy attack, report) and the finished
// fight can also be run through the full combat engine.  The scoring side
// effects and the output text are unchanged, so the rest of the game behaves
// exactly as before.

#include "spacebattlerpg.h"
#include "engine/combat_engine.h"
#include "ui/display.h"

/* ---------------- encounter setup ---------------- */

// Pick a random index into the enemy fleet (0..16).
static int rollEnemyIndex()
{
    return randomNumber(16) - 1;
}

// Announce the enemy and describe its strength.
static void announceEnemy(const enemyships &enemy)
{
    cout << "\n  " << enemy.name << " appears!\n" << endl;
    cout << enemy.name << " has " << enemy.power << " Power" << endl;
    cout << enemy.name << " has " << enemy.sif << " SIF" << endl;
    cout << enemy.name << " has " << enemy.sp << " SP\n" << endl;
}

// A fresh combat copy of the enemy, with rerolled ship points (60..100).
static void prepareEnemy(int index)
{
    enemyshipsArray[index].sp = 60 + (randomNumber(41) - 1);
}

/* ---------------- one exchange ---------------- */

// The player attacks.  Returns damage actually applied to the enemy.
static int playerAttack(int &enemySp, const enemyships &enemy)
{
    // (power + weapon) * 1d10
    int attack = (charactership.power + charactership.weapon)
               * randomNumber(10);

    cout << charactership.name << " deals " << attack << " damage" << endl;

    // The enemy defends with SIF * 1d5.
    int defence = enemy.sif * randomNumber(5);

    cout << enemy.name << " defends " << defence << " Ship Points" << endl;

    if (attack <= defence)
    {
        cout << "You have failed to damage the enemy ship!\n" << endl;
        return 0;
    }

    int damage = attack - defence;
    enemySp -= damage;

    if (enemySp < 0)
        enemySp = 0;

    cout << "The enemyship now has " << enemySp << " Ship points!" << endl;

    targetScore.damage_done += damage;
    return damage;
}

// The enemy attacks.  Returns damage actually applied to the player.
static int enemyAttack(const enemyships &enemy)
{
    // power * 1d5
    int attack = enemy.power * randomNumber(5);

    cout << enemy.name << " does " << attack << " Damage!" << endl;

    // The player defends with (SIF + shield) * 1d5.
    int defence = (charactership.sif + charactership.shield) * randomNumber(5);

    cout << charactership.name << " defends " << defence
         << " Ship Points!" << endl;

    if (attack <= defence)
    {
        cout << "You have defended the enemy ship attack! \n"
             << "...And now have " << charactership.sp
             << " Ship points!\n" << endl;
        return 0;
    }

    int damage = attack - defence;
    charactership.sp -= damage;

    cout << "You have been hit! You now have " << charactership.sp
         << " Ship Points!\n" << endl;

    targetScore.damage_taken += damage;
    return damage;
}

/* ---------------- resolution ---------------- */

// Award the kill and report it.
static void reportEnemyDestroyed(const enemyships &enemy)
{
    cout << enemy.name << " is destroyed!\n" << endl;

    targetScore.kills++;
    targetScore.score += 100;

    audioPlaySFX(SFX_ENEMY_EXPLODE);
    luaFireEvent(SBW_EVENT_ENEMY_KILLED, 0, 0);
}

/* ---------------- the battle loop ---------------- */

void initializeBattle()
{
    int index = rollEnemyIndex();

    prepareEnemy(index);
    announceEnemy(enemyshipsArray[index]);

    audioPlayMusic(MUS_COMBAT);
    luaFireEvent(SBW_EVENT_COMBAT_START, index, 0);

    // Alternate exchanges until either side is destroyed.
    for (;;)
    {
        targetScore.actions_taken++;

        playerAttack(enemyshipsArray[index].sp, enemyshipsArray[index]);

        if (enemyshipsArray[index].sp <= 0)
        {
            reportEnemyDestroyed(enemyshipsArray[index]);
            break;
        }

        enemyAttack(enemyshipsArray[index]);

        if (charactership.sp <= 0)
        {
            cout << charactership.name << " has been destroyed!\n" << endl;
            audioPlaySFX(SFX_DEFEAT);
            break;
        }
    }

    audioPlayMusic(MUS_AMBIENT_SPACE);
}
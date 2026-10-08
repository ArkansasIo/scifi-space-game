// src/ui/hud.c++ -- in-game heads-up display.

#include "spacebattlerpg.h"
#include "ui/display.h"
#include "ui/hud.h"

void showExplorationHUD(const char *locationName, int hull, int maxHull,
                        int power, int maxPower)
{
    drawRule('=');

    cout << "  Location: " << (locationName ? locationName : "deep space")
         << endl;
    cout << endl;

    drawHealthBar(hull, maxHull);
    drawPowerBar(power, maxPower);

    drawRule('-');
    out("F forward   B backward   S starboard   P port");
    out("C stats     I inventory  Q quit");
    drawRule('=');
}

void showCombatHUD(const char *playerName, int playerHp, int playerMaxHp,
                   const char *enemyName, int enemyHp, int enemyMaxHp)
{
    drawRule('=');

    cout << "  " << (playerName ? playerName : "Your ship") << endl;
    drawHealthBar(playerHp, playerMaxHp);

    cout << endl;
    cout << "  " << (enemyName ? enemyName : "Enemy") << endl;
    drawHealthBar(enemyHp, enemyMaxHp);

    drawRule('=');
}

void showScoreStrip(int score, int kills, int damageDone, int damageTaken)
{
    cout << "  Score " << score
         << "   Kills " << kills
         << "   Dealt " << damageDone
         << "   Taken " << damageTaken << endl;
}

void showModifierStrip()
{
    cout << "  Active modifiers:" << endl;
    out("(none applied)");
}

void showInventoryPanel()
{
    drawHeader("Inventory");

    drawFieldText("Weapon", charactership.weapon ? "equipped" : "none");
    drawFieldText("Armour", charactership.armour ? "equipped" : "none");
    drawFieldText("Shield", charactership.shield ? "equipped" : "none");

    cout << endl;
}

void showMapPanel(int locationIndex)
{
    drawHeader("Nav");

    if (locationIndex < 0 || locationIndex >= 17)
    {
        out("No location data.");
        return;
    }

    const space &here = spaceArray[locationIndex];

    drawFieldText("System", here.name);
    outWrapped(here.description);

    cout << "  Exits:";
    if (here.forward >= 0)
        cout << " forward(" << spaceArray[here.forward].name << ")";
    if (here.backward >= 0)
        cout << " backward(" << spaceArray[here.backward].name << ")";
    if (here.starboard >= 0)
        cout << " starboard(" << spaceArray[here.starboard].name << ")";
    if (here.port >= 0)
        cout << " port(" << spaceArray[here.port].name << ")";
    cout << endl
         << endl;
}

void showCharacterPanel()
{
    drawHeader("Character");

    drawFieldText("Name", charactership.name);
    drawField("Level", charactership.level);
    drawField("Power", charactership.power);
    drawField("SIF", charactership.sif);
    drawField("Health", charactership.health);
    drawField("Ship Points", charactership.sp);
    drawField("Experience", charactership.exp);
    drawField("Attack Power", charactership.attackpower);
    drawField("Defence Power", charactership.defencepower);

    cout << endl;
}

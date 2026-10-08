// difficultmode.c++ -- the game difficulty setting.

#include "spacebattlerpg.h"

// Easy, Normal, Hard, Very Hard, Survivor, Expert, Heroic, Hardcore,
// Impossible, Ultra Nightmare, Legendary, Prestige.
static const struct
{
    const char *name;
    int level;
} difficulties[] = {
    {"Easy", 40},
    {"Normal", 50},
    {"Hard", 60},
    {"Very Hard", 70},
    {"Survivor", 80},
    {"Expert", 90},
    {"Heroic", 100},
    {"Hardcore", 110},
    {"Impossible", 120},
    {"Ultra Nightmare", 130},
    {"Legendary", 140},
    {"Prestige", 15}};

void initializdifficultmode()
{
    int choice = 0;

    cout << "Please pick a difficulty:\n";
    for (int i = 0; i < 12; ++i)
        cout << (i + 1) << " - " << difficulties[i].name
             << " (level " << difficulties[i].level << ")\n";

    choice = readInt("Pick your difficulty: ");

    if (choice < 1 || choice > 12)
    {
        cout << "Error - Invalid input; only 1 to 12 allowed. Defaulting to Normal.\n";
        choice = 2;
    }

    strncpy(difficultMode.name, difficulties[choice - 1].name,
            sizeof(difficultMode.name) - 1);
    difficultMode.name[sizeof(difficultMode.name) - 1] = '\0';
    difficultMode.level = difficulties[choice - 1].level;

    cout << "You picked " << difficultMode.name << " difficulty.\n";
}
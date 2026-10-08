// src/game/save.c++ -- save/load/install helpers, race selection and inventory.

#include "spacebattlerpg.h"

/* ---------------- save / load / install ---------------- */

void initializesavegamedata()
{
    cout << "Save game data initialized." << endl;
}

void initializeloadgamedata()
{
    cout << "Load game data initialized." << endl;
}

void initializeinstallgamedata()
{
    cout << "Base game data installed." << endl;
}

/* ---------------- race selection ---------------- */

static const char *races[] = {
    "Human", "Klingon", "Vulcans", "Romulans", "Cardassian", "Borg",
    "Jem'Hadar", "Ferengi", "Asgard", "Goa'uld", "Tok'ra", "Jaffa",
    "Unas", "Replicator", "Asurans", "Tau'ri", "Ori", "Ancients",
    "Nox", "Reetou", "Wraith", "Cylons", "Vorlons", "Shadows",
    "Luxans", "Scarrans"};

void pickrace()
{
    const int count = (int)(sizeof(races) / sizeof(races[0]));

    cout << "\nPlease pick your race: \n";
    for (int i = 0; i < count; ++i)
        cout << (i + 1) << " - " << races[i] << "\n";

    int pickRace = readInt("Pick your race: ");

    if (pickRace < 1 || pickRace > count)
    {
        cout << "Error - Invalid input; only 1 to " << count << " allowed.\n";
        return;
    }

    cout << "You picked the " << races[pickRace - 1] << " race.\n";
}

/* ---------------- inventory ---------------- */

void initializeinventory()
{
    cout << "\n===== INVENTORY =====" << endl;
    cout << "Weapon:  " << charactership.weapon << endl;
    cout << "Armour:  " << charactership.armour << endl;
    cout << "Shield:  " << charactership.shield << endl;
    cout << "=====================\n"
         << endl;
}
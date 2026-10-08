// gaveitem.c++ -- the item random generator.  Every turn there is a chance the
// player is offered a piece of technology, an artifact, a glyph or an
// enchantment.

#include "spacebattlerpg.h"

// Ask the player yes/no and return true if they accepted.
// Named `confirmYesNo` rather than `askYesNo` so it does not collide with the
// shared UI helper of that name in include/ui/display.h.
static bool confirmYesNo(const char *question)
{
    cout << question << endl;
    cout << "Yes or No." << endl;
    return (readChar("Answer: ") == 'Y');
}

// Offer one random item out of the given table.
static void offerItem(ship_item table[], int count, const char *label)
{
    int index = rand() % count;
    ship_item &item = table[index];

    cout << "\nDo you want to get the new " << label << " " << item.name << " ?" << endl;

    bool isWeapon = (strcmp(item.type, "Weapon") == 0);
    bool isArmour = (strcmp(item.type, "Armour") == 0);

    if (isWeapon)
        cout << "This will add " << item.bonus << " to your Power." << endl;
    else if (isArmour)
        cout << "This will add " << item.bonus << " to your SIF." << endl;
    else
        cout << "This will add " << item.bonus << " to your Shields." << endl;

    if (confirmYesNo("Accept it?"))
    {
        if (isWeapon)
            charactership.weapon = item.bonus;
        else if (isArmour)
            charactership.armour = item.bonus;
        else
            charactership.shield = item.bonus;

        cout << "You have taken the " << label << " " << item.name << "!" << endl;
    }
    else
    {
        cout << "You left the " << label << " behind." << endl;
    }
}

void giveItem()
{
    /* The Technology Randomgenerator */
    int randomgenerator = 0;
    randomgenerator = rand() % 17;

    // Roughly a third of the time the player gets offered something.
    if (randomgenerator < 6)
    {
        switch (randomgenerator % 3)
        {
            case 0: offerItem(artifactArray,    17, "artifact");    break;
            case 1: offerItem(glyphsArray,      17, "glyph");       break;
            default: offerItem(enchantmentArray, 17, "enchantment"); break;
        }
    }
}
// src/data/gear.c++ -- the PvE / PvP item catalogue, sets and slots.
// See docs/FRAMEWORK.md s.17.
//
// Every item carries two stat tables.  The PvE table is the power fantasy;
// the PvP table is normalized, dampened and proc-free.

#include "spacebattlerpg.h"

#define PVE_LINE(label, raw, weight) {label, raw, weight, 0}
#define PVP_LINE(label, raw, weight) {label, raw, weight, 0}

// Weapons ---------------------------------------------------------------
static const itemdata weaponSeeds[] = {
    {"Ashen Blade", FAMILY_WEAPON, SLOT_WEAPON_MAIN, RARITY_EPIC, 40, 1, {PVE_LINE("ATK", 1200, 100), PVE_LINE("Fire Damage", 120, 130), PVE_LINE("STR", 50, 100), PVE_LINE("Crit Chance", 5, 250)}, 4, {PVP_LINE("ATK", 650, 100), PVP_LINE("PvP Power", 10, 300), PVP_LINE("Anti-Heal", 20, 150)}, 3, 90, 0, 0, 0},

    {"Void Lance", FAMILY_WEAPON, SLOT_WEAPON_MAIN, RARITY_LEGENDARY, 60, 1, {PVE_LINE("ATK", 1450, 100), PVE_LINE("Void Damage", 180, 140), PVE_LINE("Armor Pen", 12, 200), PVE_LINE("Execute Dmg", 15, 175)}, 4, {PVP_LINE("ATK", 700, 100), PVP_LINE("Shield Pen", 15, 220), PVP_LINE("Resource Drain", 10, 180), PVP_LINE("CC Strength", 6, 300)}, 4, 0, 0, 0, 0},

    {"Arc Welder", FAMILY_WEAPON, SLOT_WEAPON_MAIN, RARITY_RARE, 25, 1, {PVE_LINE("ATK", 620, 100), PVE_LINE("Lightning Damage", 70, 130), PVE_LINE("Attack Speed", 8, 200)}, 3, {PVP_LINE("ATK", 340, 100), PVP_LINE("PvP Power", 6, 300)}, 2, 0, 0, 0, 0},

    {"Point Defense Array", FAMILY_WEAPON, SLOT_WEAPON_OFF, RARITY_UNCOMMON, 15, 1, {PVE_LINE("ATK", 300, 100), PVE_LINE("Accuracy", 10, 80)}, 2, {PVP_LINE("ATK", 180, 100), PVP_LINE("Accuracy", 5, 80)},
    2,
    0, 0, 0, 0},

    /* Armor --------------------------------------------------------- */
    {"Aegis Plate", FAMILY_ARMOR, SLOT_CHEST, RARITY_EPIC, 40, 1, {PVE_LINE("Armor", 900, 75), PVE_LINE("Max HP", 1500, 85), PVE_LINE("Fire Resist", 60, 90)}, 3, {PVP_LINE("Armor", 950, 75), PVP_LINE("Resilience", 80, 300), PVP_LINE("Tenacity", 60, 200)}, 3, 0, 0, 0, 0},

    {"Thermal Visor", FAMILY_ARMOR, SLOT_HEAD, RARITY_RARE, 25, 1, {PVE_LINE("Armor", 400, 75), PVE_LINE("Accuracy", 15, 80), PVE_LINE("Crit Chance", 3, 250)}, 3, {PVP_LINE("Armor", 430, 75), PVP_LINE("CC Resist", 20, 200)}, 2, 0, 0, 0, 0},

    {"Servo Greaves", FAMILY_ARMOR, SLOT_LEGS, RARITY_UNCOMMON, 15, 1, {PVE_LINE("Armor", 350, 75), PVE_LINE("Movement Speed", 5, 90)}, 2, {PVP_LINE("Armor", 380, 75), PVP_LINE("Movement Speed", 4, 90)}, 2, 0, 0, 0, 0},

    {"Mag Boots", FAMILY_ARMOR, SLOT_BOOTS, RARITY_COMMON, 5, 1, {PVE_LINE("Armor", 120, 75), PVE_LINE("Poise", 10, 90)}, 2, {PVP_LINE("Armor", 130, 75), PVP_LINE("Tenacity", 8, 200)}, 2, 0, 0, 0, 0},

    /* Trinkets ------------------------------------------------------ */
    {"Warmind Core", FAMILY_TRINKET, SLOT_TRINKET_1, RARITY_EPIC, 40, 1, {PVE_LINE("Spell Power", 400, 100), PVE_LINE("CDR", 10, 225), PVE_LINE("Boss Damage", 15, 200)}, 3, {PVP_LINE("CC Break", 1, 400), PVP_LINE("Tenacity", 70, 200)}, 2, 120, 0, 0, 0},

    {"Emergency Shield Cell", FAMILY_TRINKET, SLOT_TRINKET_2, RARITY_RARE, 25, 1, {PVE_LINE("Shield Power", 600, 100), PVE_LINE("Regen", 20, 90)}, 2, {PVP_LINE("Shield Power", 400, 100), PVP_LINE("Anti-Burst", 25, 250)}, 2, 0, 0, 0, 0},

    {"Navigator's Ring", FAMILY_TRINKET, SLOT_RING_1, RARITY_UNCOMMON, 15, 1, {PVE_LINE("Accuracy", 20, 80), PVE_LINE("Critical Chance", 2, 250)}, 2, {PVP_LINE("Accuracy", 12, 80)}, 1, 0, 0, 0, 0},

    /* Sets ---------------------------------------------------------- */
    {"Ashen Set Helm", FAMILY_ARMOR, SLOT_HEAD, RARITY_EPIC, 40, 1, {PVE_LINE("Armor", 420, 75), PVE_LINE("Fire Damage", 40, 130)}, 2, {PVP_LINE("Armor", 450, 75), PVP_LINE("Resilience", 40, 300)}, 2, 0, 400, 0, 0},

    {"Ashen Set Gloves", FAMILY_ARMOR, SLOT_GLOVES, RARITY_EPIC, 40, 1, {PVE_LINE("Armor", 300, 75), PVE_LINE("Attack Speed", 6, 200)}, 2, {PVP_LINE("Armor", 320, 75), PVP_LINE("Tenacity", 30, 200)}, 2, 0, 400, 0, 0},

    {"Ashen Set Boots", FAMILY_ARMOR, SLOT_BOOTS, RARITY_EPIC, 40, 1, {PVE_LINE("Armor", 280, 75), PVE_LINE("Movement Speed", 8, 90)}, 2, {PVP_LINE("Armor", 300, 75), PVP_LINE("Movement Speed", 6, 90)}, 2, 0, 400, 0, 0},

    {"Ashen Set Belt", FAMILY_ARMOR, SLOT_BELT, RARITY_EPIC, 40, 1, {PVE_LINE("Armor", 260, 75), PVE_LINE("Max HP", 800, 85)}, 2, {PVP_LINE("Armor", 280, 75), PVP_LINE("Tenacity", 30, 200)}, 2, 0, 400, 0, 0}};

static const int weaponSeedCount =
    (int)(sizeof(weaponSeeds) / sizeof(weaponSeeds[0]));

/* ---------------- sets ---------------- */

static const gearsets setSeeds[] = {
    {"Ashen Warband", 4, {SLOT_HEAD, SLOT_GLOVES, SLOT_BOOTS, SLOT_BELT, SLOT_COUNT, SLOT_COUNT, SLOT_COUNT, SLOT_COUNT}, 250, 600, 1200},

    {"Warmind Arsenal", 3, {SLOT_WEAPON_MAIN, SLOT_TRINKET_1, SLOT_CHEST, SLOT_COUNT, SLOT_COUNT, SLOT_COUNT, SLOT_COUNT, SLOT_COUNT}, 180, 420, 0},

    {"Rim Runner", 2, {SLOT_CLOAK, SLOT_BOOTS, SLOT_COUNT, SLOT_COUNT, SLOT_COUNT, SLOT_COUNT, SLOT_COUNT, SLOT_COUNT}, 120, 0, 0}};

static const int setSeedCount = (int)(sizeof(setSeeds) / sizeof(setSeeds[0]));

/* ---------------- weapon / armor type names ---------------- */

static const char *weaponTypeNames[] = {
    "Greatsword", "Longsword", "Axe", "Mace", "Spear", "Dagger",
    "Fist", "Scythe", "Bow", "Crossbow", "Gun", "Throwing",
    "Staff", "Wand", "Tome", "Catalyst", "Relic"};

static const char *armorTypeNames[] = {
    "Light", "Medium", "Heavy"};

/* ---------------- initializers ---------------- */

void initializeGear()
{
    itemCount = 0;

    for (int i = 0; i < weaponSeedCount && i < MAX_ITEMS; ++i)
    {
        itemArray[itemCount] = weaponSeeds[i];
        recalcItemPower(itemArray[itemCount]);
        itemCount++;
    }
}

void initializeSets()
{
    setCount = 0;

    for (int i = 0; i < setSeedCount && i < MAX_SETS; ++i)
    {
        setArray[setCount] = setSeeds[i];
        setCount++;
    }
}

/* ---------------- lookups ---------------- */

int findItem(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < itemCount; ++i)
    {
        if (strcmp(itemArray[i].name, name) == 0)
            return i;
    }

    return -1;
}

int listItemsForSlot(gearslot slot, int out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    int written = 0;

    for (int i = 0; i < itemCount && written < outMax; ++i)
    {
        if (itemArray[i].slot == slot)
        {
            out[written] = i;
            written++;
        }
    }

    return written;
}

int listItemsInFamily(gearfamily family, int out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    int written = 0;

    for (int i = 0; i < itemCount && written < outMax; ++i)
    {
        if (itemArray[i].family == family)
        {
            out[written] = i;
            written++;
        }
    }

    return written;
}

int setForItem(int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= itemCount)
        return -1;

    // A set claims an item when the item's slot appears in the set's slot list
    // and the item carries a set bonus value.
    if (itemArray[itemIndex].setBonusValuePU <= 0)
        return -1;

    for (int s = 0; s < setCount; ++s)
    {
        for (int p = 0; p < setArray[s].pieceCount && p < 8; ++p)
        {
            if (setArray[s].pieces[p] == itemArray[itemIndex].slot)
                return s;
        }
    }

    return -1;
}

int equippedSetPieces(const gearslot slots[], int count, int setIndex)
{
    if (!slots || count <= 0)
        return 0;

    if (setIndex < 0 || setIndex >= setCount)
        return 0;

    int worn = 0;

    for (int p = 0; p < setArray[setIndex].pieceCount && p < 8; ++p)
    {
        for (int i = 0; i < count; ++i)
        {
            if (slots[i] == setArray[setIndex].pieces[p])
                worn++;
        }
    }

    return worn;
}

int setBonusForPieces(int setIndex, int pieces)
{
    if (setIndex < 0 || setIndex >= setCount)
        return 0;

    const gearsets &set = setArray[setIndex];

    // Tiers are 2 / 4 / 6 pieces; the highest reached tier applies.
    if (pieces >= 6 && set.bonus6 > 0)
        return set.bonus6;

    if (pieces >= 4 && set.bonus4 > 0)
        return set.bonus4;

    if (pieces >= 2 && set.bonus2 > 0)
        return set.bonus2;

    return 0;
}

int weaponTypeCount()
{
    return (int)(sizeof(weaponTypeNames) / sizeof(weaponTypeNames[0]));
}

const char *weaponTypeName(int index)
{
    if (index < 0 || index >= weaponTypeCount())
        return "";

    return weaponTypeNames[index];
}

int armorTypeCount()
{
    return (int)(sizeof(armorTypeNames) / sizeof(armorTypeNames[0]));
}

const char *armorTypeName(int index)
{
    if (index < 0 || index >= armorTypeCount())
        return "";

    return armorTypeNames[index];
}

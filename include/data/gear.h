// include/data/gear.h -- the PvE / PvP item catalogue, sets and slots.
// See docs/FRAMEWORK.md section 17.

#ifndef SBW_DATA_GEAR_H
#define SBW_DATA_GEAR_H

// Fill itemArray[] with weapons, armor and trinkets, each carrying a separate
// PvE and PvP stat table.
void initializeGear();

// Fill setArray[] with multi-piece sets and their 2/4/6 bonuses.
void initializeSets();

// Look up an item by name.  Returns index, or -1.
int findItem(const char *name);

// All items for a slot.  Returns how many were written.
int listItemsForSlot(gearslot slot, int out[], int outMax);

// Every item whose family matches.  Returns how many were written.
int listItemsInFamily(gearfamily family, int out[], int outMax);

// Which set (if any) an item belongs to.  Returns index, or -1.
int setForItem(int itemIndex);

// Count how many pieces of a set are equipped.
int equippedSetPieces(const gearslot slots[], int count, int setIndex);

// Set bonus value for the number of pieces worn (0 if no tier reached).
int setBonusForPieces(int setIndex, int pieces);

// Weapon types and armor types referenced by the catalogue.
int weaponTypeCount();
const char *weaponTypeName(int index);
int armorTypeCount();
const char *armorTypeName(int index);

#endif /* SBW_DATA_GEAR_H */

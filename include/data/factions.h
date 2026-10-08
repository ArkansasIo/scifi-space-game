// include/data/factions.h -- galaxy factions and their starting empires.
// See docs/FRAMEWORK.md section 22.

#ifndef SBW_DATA_FACTIONS_H
#define SBW_DATA_FACTIONS_H

// Fill factionArray[] with the player-facing and AI factions.
void initializeFactions();

// Find a faction by name.  Returns index, or -1.
int findFactionByName(const char *name);

// The faction the player belongs to (used for reputation and diplomacy).
int playerFactionIndex();

// Building names used by the colony layer.
int buildingCount();
const char *buildingName(int index);

// Ship class names (fighter .. flagship).
int shipClassNameCount();
const char *shipClassName(int index);

// Technology tree categories (physics, engineering, biotech, psionics).
int researchCategoryCount();
const char *researchCategoryName(int index);

#endif /* SBW_DATA_FACTIONS_H */

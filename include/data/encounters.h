// include/data/encounters.h -- ready-to-use encounter templates.
// See docs/FRAMEWORK.md section 13.

#ifndef SBW_DATA_ENCOUNTERS_H
#define SBW_DATA_ENCOUNTERS_H

// Fill encounterArray[] with open-world, dungeon and raid templates.
void initializeEncounters();

// Look up a template by name.  Returns index, or -1.
int findEncounter(const char *name);

// Pick a random template of the given kind.  Returns index, or -1.
int randomEncounterOfKind(encounterkind kind);

// Hazard names used by arena / room templates.
int hazardCount();
const char *hazardName(int index);

#endif /* SBW_DATA_ENCOUNTERS_H */

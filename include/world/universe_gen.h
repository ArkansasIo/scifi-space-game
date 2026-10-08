// include/world/universe_gen.h -- procedural universe generation.
//
// The galaxy is built from a deterministic seed:
//
//   Galaxy
//   └── Sector          (a cube of space, owns a faction bias)
//       └── Star System (a star, its planets, belts, stations, anomalies)
//           ├── Star            (spectral class drives habitability + value)
//           ├── Planet          (0..N), each with moons
//           │   └── Moon        (0..N)
//           ├── Asteroid Belt   (ore yield, patrol risk)
//           ├── Jump Gate       (links systems; the travel graph)
//           └── Anomaly / Interstellar Object
//
// Everything is rolled from one 32-bit seed, so the same seed always produces
// the same galaxy.  See docs/UNIVERSE.md.

#ifndef SBW_WORLD_UNIVERSE_GEN_H
#define SBW_WORLD_UNIVERSE_GEN_H

// ---- spectral classification of a star ----

enum starclass
{
    STAR_O = 0,   // blue, very hot, short-lived, metal poor
    STAR_B,       // blue-white
    STAR_A,       // white
    STAR_F,       // yellow-white
    STAR_G,       // yellow (Sol-like, best habitability)
    STAR_K,       // orange (long-lived, stable)
    STAR_M,       // red dwarf (most common, tidally locked worlds)
    STAR_GIANT,   // evolved giant, irradiates inner worlds
    STAR_NEUTRON, // collapsed core, extreme hazards
    STAR_BLACKHOLE,
    STAR_CLASS_COUNT
};;

// ---- planet classification ----

enum planetclass
{
    PLANET_BARREN = 0,
    PLANET_ROCKY,
    PLANET_OCEAN,
    PLANET_TERRESTRIAL,
    PLANET_DESERT,
    PLANET_TUNDRA,
    PLANET_JUNGLE,
    PLANET_TOXIC,
    PLANET_LAVA,
    PLANET_GAS_GIANT,
    PLANET_ICE_GIANT,
    PLANET_CLASS_COUNT
};;

// ---- interstellar object kinds ----

enum objectkind
{
    OBJ_NONE = 0,
    OBJ_ASTEROID_BELT,
    OBJ_COMET,
    OBJ_NEBULA,
    OBJ_PULSAR,
    OBJ_QUASAR,
    OBJ_DERELICT,      // abandoned ship / station
    OBJ_RELIC,         // precursor artefact
    OBJ_WORMHOLE,
    OBJ_JUMP_GATE,
    OBJ_MINING_FIELD,
    OBJ_PIRATE_STRONGHOLD,
    OBJ_RESEARCH_ANOMALY,
    OBJ_KIND_COUNT
};;

// ---- records ----

// A natural satellite.
struct moon
{
    char name[30];
    int radius; // 1..10 relative scale
    int resources;
    int habitability;
    int tidallyLocked;
    int hasStation;
};

// A world.  Extends the lightweight `planet` used by the 4X layer with
// orbital position, classification and its own moon list.
struct world
{
    char name[30];
    planetclass klass;
    int orbitIndex;    // 0 = innermost
    int orbitDistance; // in light-seconds
    int radius;
    int gravity;      // x100 (100 = 1.00g)
    int temperature;  // in kelvin
    int habitability; // 0..100
    int resources;
    int atmosphere;    // 0 = none
    int waterCoverage; // percent
    int ringSystem;
    int moonCount;
    moon moons[4];
};

// A star: the anchor of a system.
struct star
{
    char name[30];
    starclass klass;
    int mass;        // x100 solar masses
    int radius;      // x100 solar radii
    int temperature; // kelvin
    int luminosity;  // x100 solar luminosities
    int age;         // millions of years
};

// A traversable link between two systems.
struct jumpgate
{
    int targetSystem;
    int distance; // in light-years
    int security; // 0..100, low = pirate activity
    int scanned;
};

// An anomaly or interstellar object within a system.
struct anomaly
{
    char name[40];
    objectkind kind;
    int orbitIndex; // which orbital band it sits in
    int magnitude;  // strength / size
    int danger;     // 0..100
    int discovered;
    int reward;
};

// A sector: a labelled volume of the galaxy owning a slice of systems.
struct sector
{
    char name[30];
    int firstSystem;
    int systemCount;
    int development; // 0..100
    int danger;      // 0..100
    int ownerFaction;
};

// A full star system.
struct systemworld
{
    char name[30];
    star primary;
    int sectorIndex;
    int worldCount;
    world worlds[8];
    int gateCount;
    jumpgate gates[6];
    int anomalyCount;
    anomaly anomalies[6];
    int hasAsteroidBelt;
    int asteroidOre; // 0..100
    int hasStation;
    int stationKind; // 0 = trade, 1 = military, 2 = mining, 3 = research
    int population;
    int development;
    int danger;
    int ownerFaction;
    int visited;
};

/* ---------------- capacity ---------------- */

#define MAX_SECTORS 8
#define MAX_SYSTEMS_WORLDS 32
#define MAX_TOTAL_WORLDS 128
#define MAX_GASTRO_OBJECTS 64

/* ---------------- tables ---------------- */

extern star starArray[MAX_SYSTEMS_WORLDS];
extern systemworld systemWorldArray[MAX_SYSTEMS_WORLDS];
extern sector sectorArray[MAX_SECTORS];
extern world worldArray[MAX_TOTAL_WORLDS];
extern anomaly anomalyArray[MAX_GASTRO_OBJECTS];

extern int systemWorldCount;
extern int sectorRecordCount;
extern int worldRecordCount;
extern int anomalyRecordCount;

/* ---------------- label lookups ---------------- */

const char *starClassName(starclass klass);
const char *planetClassName(planetclass klass);
const char *objectKindName(objectkind kind);
const char *stationKindName(int kind);

/* ---------------- naming ---------------- */

// Greek designations used for stars and catalogue names.
const char *greekLetter(int index);

// Build a procedural system name from the seed.
void generateSystemName(char out[], int outSize, int index, int seed);

// Build a procedural world name from its system and orbit.
void generateWorldName(char out[], int outSize, const char *systemName,
                       int orbitIndex, int seed);

/* ---------------- generation ---------------- */

// Seed the whole universe.  The same seed always yields the same galaxy.
void generateUniverse(unsigned int seed);

// Generate one sector and everything in it.
void generateSector(sector &sec, int index, unsigned int seed);

// Generate one star system: primary, worlds, moons, belt, gates, anomalies.
void generateStarSystem(systemworld &sys, int index, int sectorIndex,
                        unsigned int seed);

// Roll the primary star of a system.
void rollStar(star &s, unsigned int seed);

// Roll one world at an orbital position around a star.
void rollWorld(world &w, int orbitIndex, const star &primary,
               unsigned int seed);

// Roll the moons of a world.
void rollMoons(world &w, unsigned int seed);

// Roll the anomalies of a system.
int rollAnomalies(systemworld &sys, unsigned int seed);

// Roll a jump gate from this system toward another.
void rollJumpGate(jumpgate &g, int fromSystem, unsigned int seed);

// Link the systems into a traversable graph.
void linkSystemGates(unsigned int seed);

/* ---------------- queries ---------------- */

// Find a system by name.  Returns index, or -1.
int findSystemWorld(const char *name);

// Total worlds generated across every system.
int countTotalWorlds();

// Habitable worlds in a system (habitability >= 40).
int countHabitableWorlds(int systemIndex);

// The richest world in a system (highest resources).  Returns index, or -1.
int richestWorldInSystem(int systemIndex);

// Systems reachable from this one in a single jump.
int reachableSystems(int systemIndex, int out[], int outMax);

// Shortest number of jumps between two systems (-1 if unreachable).
int jumpDistance(int fromSystem, int toSystem);

// The sector that owns a system.
int sectorForSystem(int systemIndex);

// Random system index weighted by development and danger.
int pickRandomSystem(unsigned int seed);

/* ---------------- classification helpers ---------------- */

// Habitability a star class allows at the given orbital position.
int habitabilityBaseForStar(starclass klass);

// The likely planet class for a temperature and star class.
planetclass classifyWorld(int temperature, starclass klass, int waterCoverage);

// Is this world class naturally colonisable by the player's faction?
int isColonisable(planetclass klass);

// Roman numeral for an orbit position (I, II, III ...).
const char *orbitNumeral(int orbitIndex);

/* ---------------- presentation ---------------- */

void showUniverseOverview();
void showSector(int sectorIndex);
void showSystemWorld(int systemIndex);
void showSystemList();
void showAnomalyList(int systemIndex);

#endif /* SBW_WORLD_UNIVERSE_GEN_H */

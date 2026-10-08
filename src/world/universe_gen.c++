// src/world/universe_gen.c++ -- procedural universe generation.
//
// Everything is derived from one seed.  A tiny linear-congruential generator
// is used rather than rand(), so a galaxy is reproducible: the same seed gives
// the same star classes, worlds, moons and gates on every machine.
//
// See docs/UNIVERSE.md for the design.

#include "spacebattlerpg.h"
#include "world/universe_gen.h"

/* ---------------- storage ---------------- */

star starArray[MAX_SYSTEMS_WORLDS];
systemworld systemWorldArray[MAX_SYSTEMS_WORLDS];
sector sectorArray[MAX_SECTORS];
world worldArray[MAX_TOTAL_WORLDS];
anomaly anomalyArray[MAX_GASTRO_OBJECTS];

int systemWorldCount = 0;
int sectorRecordCount = 0;
int worldRecordCount = 0;
int anomalyRecordCount = 0;

/* ---------------- deterministic rng ---------------- */

// A small LCG.  Deliberately not rand(): we need reproducibility across
// machines and the ability to reseed from a galaxy seed.
static unsigned int g_rngState = 1u;

static void rngSeed(unsigned int seed)
{
    g_rngState = (seed == 0u) ? 1u : seed;
}

static unsigned int rngNext()
{
    // Numerical Recipes constants.
    g_rngState = (g_rngState * 1664525u) + 1013904223u;
    return g_rngState;
}

// Uniform in [0, maxExclusive).
static int rngRange(int maxExclusive)
{
    if (maxExclusive <= 0)
        return 0;

    return (int)(rngNext() % (unsigned int)maxExclusive);
}

// Uniform in [min, max].
static int rngBetween(int minimum, int maximum)
{
    if (maximum <= minimum)
        return minimum;

    return minimum + rngRange(maximum - minimum + 1);
}

// 0..100 chance check.
static int rngChance(int percent)
{
    if (percent <= 0)
        return 0;
    if (percent >= 100)
        return 1;

    return rngRange(100) < percent;
}

/* ---------------- label tables ---------------- */

static const char *starClassNames[STAR_CLASS_COUNT] = {
    "Class O (blue)", "Class B (blue-white)", "Class A (white)",
    "Class F (yellow-white)", "Class G (yellow)", "Class K (orange)",
    "Class M (red dwarf)", "Giant", "Neutron Star", "Black Hole"};

static const char *planetClassNames[PLANET_CLASS_COUNT] = {
    "Barren", "Rocky", "Ocean", "Terrestrial", "Desert", "Tundra",
    "Jungle", "Toxic", "Lava", "Gas Giant", "Ice Giant"};

static const char *objectKindNames[OBJ_KIND_COUNT] = {
    "None", "Asteroid Belt", "Comet", "Nebula", "Pulsar", "Quasar",
    "Derelict", "Precursor Relic", "Wormhole", "Jump Gate",
    "Mining Field", "Pirate Stronghold", "Research Anomaly"};

static const char *stationKindNames[4] = {
    "Trade Station", "Military Outpost", "Mining Platform", "Research Lab"};

const char *starClassName(starclass klass)
{
    if (klass < 0 || klass >= STAR_CLASS_COUNT)
        return "Unknown";

    return starClassNames[klass];
}

const char *planetClassName(planetclass klass)
{
    if (klass < 0 || klass >= PLANET_CLASS_COUNT)
        return "Unknown";

    return planetClassNames[klass];
}

const char *objectKindName(objectkind kind)
{
    if (kind < 0 || kind >= OBJ_KIND_COUNT)
        return "Unknown";

    return objectKindNames[kind];
}

const char *stationKindName(int kind)
{
    if (kind < 0 || kind > 3)
        return "Station";

    return stationKindNames[kind];
}

const char *greekLetter(int index)
{
    static const char *letters[] = {
        "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta",
        "Theta", "Iota", "Kappa", "Lambda", "Mu", "Nu", "Xi",
        "Omicron", "Pi", "Rho", "Sigma", "Tau", "Upsilon", "Phi",
        "Chi", "Psi", "Omega"};

    int count = (int)(sizeof(letters) / sizeof(letters[0]));

    if (index < 0)
        index = -index;

    return letters[index % count];
}

const char *orbitNumeral(int orbitIndex)
{
    static const char *numerals[] = {
        "I", "II", "III", "IV", "V", "VI", "VII", "VIII",
        "IX", "X", "XI", "XII"};

    int count = (int)(sizeof(numerals) / sizeof(numerals[0]));

    if (orbitIndex < 0)
        orbitIndex = 0;

    return numerals[orbitIndex % count];
}

/* ---------------- naming ---------------- */

// Catalogues: real-ish star names, then a Greek + catalogue fallback.
static const char *baseSystemNames[] = {
    "Aaamazzara", "Altair", "Aurelia", "Bajor", "Benthos",
    "Borg Prime", "Cait", "Cardassia", "Cygnia", "Daran",
    "Duronom", "Dytallix", "Efros", "El-Adrel", "Epsilon Caneris",
    "Ferenginar", "Finnea", "Groombridge", "Helios", "Icarus",
    "Kepler", "Luyten", "Mizar", "Nyx", "Oberon",
    "Pavonis", "Quasaris", "Rigel", "Sirius", "Tantalus",
    "Umbriel", "Vega", "Wolf 359", "Xanth", "Yildun", "Zeta Reticuli"};

static const int baseSystemNameCount =
    (int)(sizeof(baseSystemNames) / sizeof(baseSystemNames[0]));

void generateSystemName(char out[], int outSize, int index, int seed)
{
    if (!out || outSize <= 0)
        return;

    if (index < baseSystemNameCount && index >= 0)
    {
        // Use the curated name, with a catalogue suffix for repeats.
        snprintf(out, outSize, "%s", baseSystemNames[index]);
        return;
    }

    // Beyond the curated list, build a catalogue designation.
    int letterIndex = (index + seed) % 24;
    int number = 1 + ((index * 7 + seed) % 400);

    snprintf(out, outSize, "%s %d", greekLetter(letterIndex), number);
}

void generateWorldName(char out[], int outSize, const char *systemName,
                       int orbitIndex, int seed)
{
    if (!out || outSize <= 0)
        return;

    const char *base = systemName ? systemName : "Unnamed";

    // Roman numeral for the orbit, with a small seeded variation so two
    // systems with the same base name do not collide.
    int variant = (seed + orbitIndex * 13) % 4;

    static const char *suffixes[] = {"", " Prime", " Minor", " Major"};

    snprintf(out, outSize, "%s %s%s",
             base, orbitNumeral(orbitIndex), suffixes[variant]);
}

/* ---------------- star generation ---------------- */

// Realistic-ish main sequence distribution: M dwarfs dominate.
static starclass rollStarClass()
{
    int roll = rngRange(1000);

    if (roll < 3)
        return STAR_BLACKHOLE;
    if (roll < 8)
        return STAR_NEUTRON;
    if (roll < 25)
        return STAR_O;
    if (roll < 55)
        return STAR_B;
    if (roll < 100)
        return STAR_A;
    if (roll < 180)
        return STAR_F;
    if (roll < 280)
        return STAR_G;
    if (roll < 400)
        return STAR_K;
    if (roll < 650)
        return STAR_GIANT;

    return STAR_M; // the other 35%
}

void rollStar(star &s, unsigned int seed)
{
    memset(&s, 0, sizeof(s));

    rngSeed(seed);

    s.klass = rollStarClass();

    // Mass, radius, temperature and luminosity are correlated with class.
    switch (s.klass)
    {
    case STAR_O:
        s.mass = rngBetween(1600, 4000);
        s.radius = rngBetween(600, 1000);
        s.temperature = rngBetween(30000, 50000);
        s.luminosity = rngBetween(3000000, 30000000);
        s.age = rngBetween(1, 10);
        break;

    case STAR_B:
        s.mass = rngBetween(200, 1600);
        s.radius = rngBetween(180, 600);
        s.temperature = rngBetween(10000, 30000);
        s.luminosity = rngBetween(2500, 3000000);
        s.age = rngBetween(10, 400);
        break;

    case STAR_A:
        s.mass = rngBetween(150, 210);
        s.radius = rngBetween(140, 180);
        s.temperature = rngBetween(7500, 10000);
        s.luminosity = rngBetween(1500, 2500);
        s.age = rngBetween(400, 1500);
        break;

    case STAR_F:
        s.mass = rngBetween(110, 150);
        s.radius = rngBetween(120, 140);
        s.temperature = rngBetween(6000, 7500);
        s.luminosity = rngBetween(300, 1500);
        s.age = rngBetween(1500, 5000);
        break;

    case STAR_G:
        s.mass = rngBetween(90, 110);
        s.radius = rngBetween(90, 120);
        s.temperature = rngBetween(5200, 6000);
        s.luminosity = rngBetween(80, 300);
        s.age = rngBetween(4000, 10000);
        break;

    case STAR_K:
        s.mass = rngBetween(50, 90);
        s.radius = rngBetween(60, 90);
        s.temperature = rngBetween(3700, 5200);
        s.luminosity = rngBetween(15, 80);
        s.age = rngBetween(10000, 25000);
        break;

    case STAR_M:
        s.mass = rngBetween(8, 50);
        s.radius = rngBetween(10, 60);
        s.temperature = rngBetween(2400, 3700);
        s.luminosity = rngBetween(1, 15);
        s.age = rngBetween(20000, 100000);
        break;

    case STAR_GIANT:
        s.mass = rngBetween(80, 800);
        s.radius = rngBetween(1000, 5000);
        s.temperature = rngBetween(3000, 5000);
        s.luminosity = rngBetween(1000, 100000);
        s.age = rngBetween(100, 2000);
        break;

    case STAR_NEUTRON:
        s.mass = rngBetween(140, 210);
        s.radius = 1;
        s.temperature = rngBetween(600000, 1000000);
        s.luminosity = rngBetween(1, 100);
        s.age = rngBetween(1000, 100000);
        break;

    case STAR_BLACKHOLE:
    default:
        s.mass = rngBetween(300, 3000);
        s.radius = 3;
        s.temperature = 0;
        s.luminosity = 0;
        s.age = rngBetween(10000, 1000000);
        break;
    }
}

/* ---------------- world generation ---------------- */

int habitabilityBaseForStar(starclass klass)
{
    switch (klass)
    {
    case STAR_G:
        return 60; // Sol-like: the sweet spot
    case STAR_K:
        return 50; // long-lived and stable
    case STAR_F:
        return 45;
    case STAR_M:
        return 30; // habitable zone is close, worlds tidally locked
    case STAR_A:
        return 25;
    case STAR_GIANT:
        return 15; // the habitable zone has moved outward
    case STAR_B:
        return 10;
    case STAR_O:
        return 5;
    case STAR_NEUTRON:
        return 2;
    case STAR_BLACKHOLE:
    default:
        return 0;
    }
}

planetclass classifyWorld(int temperature, starclass klass, int waterCoverage)
{
    // Ice and gas giants are decided by temperature and position, not rock.
    if (temperature < 120 && waterCoverage > 60)
        return PLANET_ICE_GIANT;

    if (temperature > 900)
        return PLANET_GAS_GIANT;

    if (temperature > 500)
        return PLANET_LAVA;

    if (temperature > 700)
        return PLANET_TOXIC;

    if (temperature < 180)
        return PLANET_TUNDRA;

    if (temperature < 260)
        return PLANET_BARREN;

    if (waterCoverage > 70)
        return PLANET_OCEAN;

    if (temperature > 340)
        return PLANET_DESERT;

    if (waterCoverage > 35)
    {
        if (klass == STAR_M && waterCoverage > 55)
            return PLANET_JUNGLE;

        return PLANET_TERRESTRIAL;
    }

    return PLANET_ROCKY;
}

int isColonisable(planetclass klass)
{
    // Only the classes with a workable atmosphere and temperature band.
    return (klass == PLANET_TERRESTRIAL || klass == PLANET_OCEAN || klass == PLANET_DESERT || klass == PLANET_TUNDRA || klass == PLANET_JUNGLE);
}

void rollWorld(world &w, int orbitIndex, const star &primary,
               unsigned int seed)
{
    memset(&w, 0, sizeof(w));

    rngSeed(seed + (unsigned)(orbitIndex * 7919));

    w.orbitIndex = orbitIndex;

    // Kepler-ish spacing: orbits grow geometrically outward.
    w.orbitDistance = 30 + (orbitIndex * orbitIndex * 12) + rngRange(20);

    w.radius = rngBetween(20, 700);  // x10 km-ish scale
    w.gravity = rngBetween(20, 280); // 0.20g .. 2.80g
    w.ringSystem = rngChance(15);

    // Temperature falls off with distance and rises with the star's output.
    int starHeat = primary.temperature / 100;

    w.temperature = 1000 - (w.orbitDistance * 2) + starHeat;
    w.temperature += rngBetween(-80, 80);

    if (w.temperature < 3)
        w.temperature = 3;

    w.waterCoverage = rngRange(101);
    w.klass = classifyWorld(w.temperature, primary.klass, w.waterCoverage);

    // Habitability: the star's ceiling, reduced by temperature and gravity.
    int base = habitabilityBaseForStar(primary.klass);

    if (w.temperature >= 240 && w.temperature <= 320)
        base += 25;
    else if (w.temperature >= 200 && w.temperature <= 360)
        base += 10;
    else
        base -= 25;

    if (w.gravity >= 60 && w.gravity <= 160)
        base += 10;
    else
        base -= 20;

    w.habitability = base + rngBetween(-10, 10);
    if (w.habitability < 0)
        w.habitability = 0;
    if (w.habitability > 100)
        w.habitability = 100;

    // Atmosphere scales with gravity and habitability.
    w.atmosphere = (w.gravity / 10) + (w.habitability / 20);

    // Resources: richer further out, and on volcanic or rocky bodies.
    w.resources = rngBetween(10, 70);

    if (w.klass == PLANET_LAVA || w.klass == PLANET_ROCKY)
        w.resources += 20;

    if (w.klass == PLANET_GAS_GIANT || w.klass == PLANET_ICE_GIANT)
        w.resources += 15;

    w.resources += orbitIndex;

    if (w.resources > 100)
        w.resources = 100;

    rollMoons(w, seed + 31);
}

void rollMoons(world &w, unsigned int seed)
{
    rngSeed(seed + (unsigned)(w.orbitIndex * 104729));

    // Bigger, further-out bodies hold more moons.
    int chance = 10;

    if (w.klass == PLANET_GAS_GIANT || w.klass == PLANET_ICE_GIANT)
        chance = 85;
    else if (w.radius > 300)
        chance = 45;
    else if (w.radius > 120)
        chance = 25;

    if (!rngChance(chance))
    {
        w.moonCount = 0;
        return;
    }

    int count = 1;

    if (w.klass == PLANET_GAS_GIANT)
        count = rngBetween(2, 4);
    else if (w.klass == PLANET_ICE_GIANT)
        count = rngBetween(1, 3);
    else if (rngChance(30))
        count = 2;

    if (count > 4)
        count = 4;

    for (int i = 0; i < count; ++i)
    {
        moon &m = w.moons[i];

        rngSeed(seed + (unsigned)(i * 31337) + (unsigned)w.orbitIndex);

        snprintf(m.name, sizeof(m.name), "Moon %s", orbitNumeral(i + 1));

        m.radius = rngBetween(2, 80);
        m.resources = rngBetween(5, 60);
        m.habitability = rngBetween(0, 25);
        m.tidallyLocked = rngChance(60);
        m.hasStation = rngChance(20);
    }

    w.moonCount = count;
}

/* ---------------- anomalies and objects ---------------- */

static objectkind rollObjectKind(int danger)
{
    int roll = rngRange(100);

    // Dangerous sectors bias toward hostile objects.
    if (danger > 60)
    {
        if (roll < 20)
            return OBJ_PIRATE_STRONGHOLD;
        if (roll < 32)
            return OBJ_DERELICT;
        if (roll < 40)
            return OBJ_ASTEROID_BELT;
        if (roll < 50)
            return OBJ_MINING_FIELD;
    }

    if (roll < 18)
        return OBJ_ASTEROID_BELT;
    if (roll < 30)
        return OBJ_COMET;
    if (roll < 40)
        return OBJ_NEBULA;
    if (roll < 48)
        return OBJ_DERELICT;
    if (roll < 56)
        return OBJ_MINING_FIELD;
    if (roll < 64)
        return OBJ_RESEARCH_ANOMALY;
    if (roll < 70)
        return OBJ_RELIC;
    if (roll < 76)
        return OBJ_WORMHOLE;
    if (roll < 82)
        return OBJ_PULSAR;
    if (roll < 88)
        return OBJ_QUASAR;

    return OBJ_JUMP_GATE;
}

int rollAnomalies(systemworld &sys, unsigned int seed)
{
    rngSeed(seed + 777);

    // Dangerous, undeveloped systems hold more to find.
    int count = rngBetween(0, 2);

    if (sys.danger > 50 && rngChance(40))
        count++;

    if (count > 6)
        count = 6;

    static const char *prefixes[] = {
        "Anomaly", "Signal", "Wreck", "Field", "Relic", "Beacon"};

    int written = 0;

    for (int i = 0; i < count; ++i)
    {
        if (anomalyRecordCount >= MAX_GASTRO_OBJECTS)
            break;

        anomaly a;
        memset(&a, 0, sizeof(a));

        a.kind = rollObjectKind(sys.danger);
        a.orbitIndex = rngRange(12);

        snprintf(a.name, sizeof(a.name), "%s %s-%d",
                 objectKindName(a.kind),
                 prefixes[rngRange(6)],
                 1 + rngRange(99));

        a.magnitude = rngBetween(1, 100);
        a.danger = rngBetween(0, 100);
        a.reward = rngBetween(10, 500) * (a.magnitude / 20 + 1);
        a.discovered = 0;

        // Store on the system, and in the global roll-up.
        sys.anomalies[i] = a;
        anomalyArray[anomalyRecordCount] = a;

        anomalyRecordCount++;
        written++;
    }

    sys.anomalyCount = written;

    return written;
}

/* ---------------- jump gates ---------------- */

void rollJumpGate(jumpgate &g, int fromSystem, unsigned int seed)
{
    memset(&g, 0, sizeof(g));

    rngSeed(seed + (unsigned)(fromSystem * 2654435761u));

    // The caller fills in targetSystem; distance is a plausibility value.
    g.distance = rngBetween(2, 40);
    g.security = rngBetween(5, 95);
    g.scanned = 0;
}

void linkSystemGates(unsigned int seed)
{
    // Build a connected graph: a ring for guaranteed reachability, then extra
    // chords for interesting routing, biased toward nearby systems.
    for (int i = 0; i < systemWorldCount; ++i)
    {
        systemworld &sys = systemWorldArray[i];

        sys.gateCount = 0;

        int next = (i + 1) % systemWorldCount;
        int prev = (i - 1 + systemWorldCount) % systemWorldCount;

        jumpgate forward;
        rollJumpGate(forward, i, seed + (unsigned)i);
        forward.targetSystem = next;
        sys.gates[sys.gateCount] = forward;
        sys.gateCount++;

        jumpgate back;
        rollJumpGate(back, i, seed + (unsigned)(i + 5000));
        back.targetSystem = prev;
        sys.gates[sys.gateCount] = back;
        sys.gateCount++;

        // Two optional chords to systems 2-5 ahead.
        for (int c = 0; c < 2; ++c)
        {
            rngSeed(seed + (unsigned)(i * 97 + c * 31));

            if (!rngChance(45))
                continue;

            int offset = 2 + rngRange(4);
            int target = (i + offset) % systemWorldCount;

            if (target == i || sys.gateCount >= 6)
                continue;

            // Skip duplicates.
            int duplicate = 0;

            for (int g = 0; g < sys.gateCount; ++g)
            {
                if (sys.gates[g].targetSystem == target)
                    duplicate = 1;
            }

            if (duplicate)
                continue;

            jumpgate chord;
            rollJumpGate(chord, i, seed + (unsigned)(i * 13 + c));
            chord.targetSystem = target;
            sys.gates[sys.gateCount] = chord;
            sys.gateCount++;
        }
    }
}

/* ---------------- systems ---------------- */

void generateStarSystem(systemworld &sys, int index, int sectorIndex,
                        unsigned int seed)
{
    memset(&sys, 0, sizeof(sys));

    rngSeed(seed + (unsigned)(index * 1000003u));

    generateSystemName(sys.name, sizeof(sys.name), index, (int)seed);

    sys.sectorIndex = sectorIndex;

    // The primary star.
    rollStar(sys.primary, seed + (unsigned)(index * 31 + 1));

    // Stars, giants and collapsed objects support different numbers of worlds.
    int maxWorlds = 8;

    if (sys.primary.klass == STAR_BLACKHOLE)
        maxWorlds = 3;
    else if (sys.primary.klass == STAR_NEUTRON)
        maxWorlds = 2;
    else if (sys.primary.klass == STAR_GIANT)
        maxWorlds = 5;

    int worldCount = rngBetween(1, maxWorlds);

    sys.worldCount = 0;

    for (int i = 0; i < worldCount; ++i)
    {
        if (worldRecordCount >= MAX_TOTAL_WORLDS)
            break;

        world w;
        rollWorld(w, i, sys.primary,
                  seed + (unsigned)(index * 1000 + i * 17));

        generateWorldName(w.name, sizeof(w.name), sys.name, i, (int)seed);

        // A tidally locked inner world on an M dwarf is a common case.
        if (sys.primary.klass == STAR_M && i == 0 && rngChance(60))
            w.habitability = w.habitability / 2;

        sys.worlds[sys.worldCount] = w;
        worldArray[worldRecordCount] = w;

        sys.worldCount++;
        worldRecordCount++;
    }

    // Asteroid belt: common around old, metal-rich stars.
    sys.hasAsteroidBelt = rngChance(55);
    sys.asteroidOre = rngBetween(10, 95);

    // A station, if there is anywhere worth stopping.
    sys.hasStation = rngChance(45);
    sys.stationKind = rngRange(4);

    // Development and danger: settled systems are safer and richer.
    sys.development = rngBetween(0, 100);
    sys.population = sys.development * rngBetween(50, 500);
    sys.danger = 100 - sys.development;

    if (sys.primary.klass == STAR_BLACKHOLE || sys.primary.klass == STAR_NEUTRON)
        sys.danger += 40;

    if (sys.danger > 100)
        sys.danger = 100;

    sys.ownerFaction = -1;
    sys.visited = 0;

    rollAnomalies(sys, seed + (unsigned)(index * 53 + 9));
}

/* ---------------- sectors ---------------- */

static const char *sectorNames[MAX_SECTORS] = {
    "Core Worlds", "Inner Rim", "Orion Spur", "Perseus Arm",
    "Outer Rim", "The Veil", "Dark Reach", "The Verge"};

void generateSector(sector &sec, int index, unsigned int seed)
{
    memset(&sec, 0, sizeof(sec));

    if (index < 0 || index >= MAX_SECTORS)
        return;

    strncpy(sec.name, sectorNames[index], sizeof(sec.name) - 1);
    sec.name[sizeof(sec.name) - 1] = '\0';

    // The core is developed and safe; the frontier is the opposite.
    sec.development = 100 - (index * 12);
    if (sec.development < 5)
        sec.development = 5;

    sec.danger = 10 + (index * 12);
    if (sec.danger > 95)
        sec.danger = 95;

    sec.ownerFaction = (index < 3) ? 0 : -1; // the Federation holds the core

    (void)seed;
}

/* ---------------- the whole universe ---------------- */

void generateUniverse(unsigned int seed)
{
    systemWorldCount = 0;
    sectorRecordCount = 0;
    worldRecordCount = 0;
    anomalyRecordCount = 0;

    memset(systemWorldArray, 0, sizeof(systemWorldArray));
    memset(sectorArray, 0, sizeof(sectorArray));
    memset(worldArray, 0, sizeof(worldArray));
    memset(anomalyArray, 0, sizeof(anomalyArray));
    memset(starArray, 0, sizeof(starArray));

    // Build the sectors.
    for (int s = 0; s < MAX_SECTORS; ++s)
    {
        generateSector(sectorArray[s], s, seed + (unsigned)(s * 7));
        sectorRecordCount++;
    }

    // Build MAX_SYSTEMS_WORLDS systems, distributed across the sectors.
    int systemsPerSector = MAX_SYSTEMS_WORLDS / MAX_SECTORS;

    if (systemsPerSector < 1)
        systemsPerSector = 1;

    for (int i = 0; i < MAX_SYSTEMS_WORLDS; ++i)
    {
        int sectorIndex = i / systemsPerSector;

        if (sectorIndex >= MAX_SECTORS)
            sectorIndex = MAX_SECTORS - 1;

        generateStarSystem(systemWorldArray[i], i, sectorIndex,
                           seed + (unsigned)(i * 101 + 7));

        starArray[i] = systemWorldArray[i].primary;
        systemWorldCount++;
    }

    // Assign sector ranges now that we know how many systems each holds.
    for (int s = 0; s < MAX_SECTORS; ++s)
    {
        sectorArray[s].firstSystem = s * systemsPerSector;
        sectorArray[s].systemCount = systemsPerSector;
    }

    // Wad the last sector out if the division left a remainder.
    sectorArray[MAX_SECTORS - 1].systemCount =
        MAX_SYSTEMS_WORLDS - sectorArray[MAX_SECTORS - 1].firstSystem;

    // Then connect them.
    linkSystemGates(seed);
}

/* ---------------- queries ---------------- */

int findSystemWorld(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < systemWorldCount; ++i)
    {
        if (strcmp(systemWorldArray[i].name, name) == 0)
            return i;
    }

    return -1;
}

int countTotalWorlds()
{
    int total = 0;

    for (int i = 0; i < systemWorldCount; ++i)
        total += systemWorldArray[i].worldCount;

    return total;
}

int countHabitableWorlds(int systemIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return 0;

    const systemworld &sys = systemWorldArray[systemIndex];
    int count = 0;

    for (int i = 0; i < sys.worldCount && i < 8; ++i)
    {
        if (sys.worlds[i].habitability >= 40)
            count++;
    }

    return count;
}

int richestWorldInSystem(int systemIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return -1;

    const systemworld &sys = systemWorldArray[systemIndex];

    int best = -1;
    int bestResources = -1;

    for (int i = 0; i < sys.worldCount && i < 8; ++i)
    {
        if (sys.worlds[i].resources > bestResources)
        {
            bestResources = sys.worlds[i].resources;
            best = i;
        }
    }

    return best;
}

int reachableSystems(int systemIndex, int out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return 0;

    const systemworld &sys = systemWorldArray[systemIndex];

    int written = 0;

    for (int i = 0; i < sys.gateCount && written < outMax; ++i)
    {
        int target = sys.gates[i].targetSystem;

        if (target >= 0 && target < systemWorldCount)
        {
            out[written] = target;
            written++;
        }
    }

    return written;
}

int jumpDistance(int fromSystem, int toSystem)
{
    if (fromSystem < 0 || fromSystem >= systemWorldCount)
        return -1;

    if (toSystem < 0 || toSystem >= systemWorldCount)
        return -1;

    if (fromSystem == toSystem)
        return 0;

    // Breadth-first search over the gate graph.
    int distance[MAX_SYSTEMS_WORLDS];
    int queue[MAX_SYSTEMS_WORLDS];
    int head = 0;
    int tail = 0;

    for (int i = 0; i < MAX_SYSTEMS_WORLDS; ++i)
        distance[i] = -1;

    distance[fromSystem] = 0;
    queue[tail++] = fromSystem;

    while (head < tail)
    {
        int current = queue[head++];

        if (current == toSystem)
            return distance[current];

        const systemworld &sys = systemWorldArray[current];

        for (int g = 0; g < sys.gateCount; ++g)
        {
            int next = sys.gates[g].targetSystem;

            if (next < 0 || next >= systemWorldCount)
                continue;

            if (distance[next] >= 0)
                continue;

            distance[next] = distance[current] + 1;
            queue[tail++] = next;

            if (tail >= MAX_SYSTEMS_WORLDS)
                break;
        }
    }

    return -1; // unreachable
}

int sectorForSystem(int systemIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return -1;

    return systemWorldArray[systemIndex].sectorIndex;
}

int pickRandomSystem(unsigned int seed)
{
    if (systemWorldCount <= 0)
        return -1;

    rngSeed(seed + 4242);

    // Bias toward systems that are developed or dangerous: more interesting.
    for (int attempt = 0; attempt < 8; ++attempt)
    {
        int index = rngRange(systemWorldCount);

        if (index < 0 || index >= systemWorldCount)
            continue;

        int pull = systemWorldArray[index].development + systemWorldArray[index].danger;

        if (rngChance(pull / 2))
            return index;
    }

    return rngRange(systemWorldCount);
}

/* ---------------- presentation ---------------- */

void showUniverseOverview()
{
    drawHeader("Universe");

    drawField("Sectors", sectorRecordCount);
    drawField("Systems", systemWorldCount);
    drawField("Worlds", countTotalWorlds());
    drawField("Objects", anomalyRecordCount);

    int habitable = 0;

    for (int i = 0; i < systemWorldCount; ++i)
        habitable += countHabitableWorlds(i);

    drawField("Habitable worlds", habitable);

    cout << endl;

    for (int s = 0; s < sectorRecordCount; ++s)
    {
        const sector &sec = sectorArray[s];

        cout << "  " << sec.name
             << "  systems " << sec.firstSystem << "-"
             << (sec.firstSystem + sec.systemCount - 1)
             << "  dev " << sec.development
             << "  danger " << sec.danger << endl;
    }

    cout << endl;
}

void showSector(int sectorIndex)
{
    if (sectorIndex < 0 || sectorIndex >= sectorRecordCount)
    {
        out("No such sector.");
        return;
    }

    const sector &sec = sectorArray[sectorIndex];

    drawHeader(sec.name);

    drawField("Development", sec.development);
    drawField("Danger", sec.danger);

    if (sec.ownerFaction >= 0 && sec.ownerFaction < factionCount)
        drawFieldText("Owner", factionArray[sec.ownerFaction].name);

    cout << endl;

    for (int i = sec.firstSystem;
         i < sec.firstSystem + sec.systemCount && i < systemWorldCount; ++i)
    {
        const systemworld &sys = systemWorldArray[i];

        cout << "  " << i << ". " << sys.name
             << "  [" << starClassName(sys.primary.klass) << "]"
             << "  worlds " << sys.worldCount
             << "  dev " << sys.development
             << "  danger " << sys.danger << endl;
    }

    cout << endl;
}

void showSystemWorld(int systemIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
    {
        out("No such system.");
        return;
    }

    const systemworld &sys = systemWorldArray[systemIndex];

    drawHeader(sys.name);

    drawFieldText("Star", starClassName(sys.primary.klass));
    drawField("Temperature (K)", sys.primary.temperature);
    drawField("Mass (x100)", sys.primary.mass);
    drawField("Age (Myr)", sys.primary.age);
    drawField("Sector", sys.sectorIndex);
    drawField("Development", sys.development);
    drawField("Danger", sys.danger);

    if (sys.hasStation)
        drawFieldText("Station", stationKindName(sys.stationKind));

    if (sys.hasAsteroidBelt)
        drawField("Asteroid ore", sys.asteroidOre);

    cout << endl
         << "  Worlds:" << endl;

    for (int i = 0; i < sys.worldCount && i < 8; ++i)
    {
        const world &w = sys.worlds[i];

        cout << "    " << (i + 1) << ". " << w.name
             << "  (" << planetClassName(w.klass) << ")" << endl;

        cout << "       orbit " << w.orbitDistance << " ls"
             << "  gravity " << (w.gravity / 100) << "."
             << (w.gravity % 100) << "g"
             << "  temp " << w.temperature << "K"
             << "  hab " << w.habitability
             << "  res " << w.resources
             << "  moons " << w.moonCount << endl;

        if (w.moonCount > 0)
        {
            cout << "       moons:";

            for (int m = 0; m < w.moonCount && m < 4; ++m)
            {
                cout << " " << w.moons[m].name
                     << "(res " << w.moons[m].resources << ")";
            }

            cout << endl;
        }
    }

    if (sys.anomalyCount > 0)
        showAnomalyList(systemIndex);

    cout << endl;
}

void showSystemList()
{
    drawHeader("Systems");

    for (int i = 0; i < systemWorldCount; ++i)
    {
        const systemworld &sys = systemWorldArray[i];

        cout << "  " << i << ". " << sys.name
             << "  [" << starClassName(sys.primary.klass) << "]\t"
             << "  worlds " << sys.worldCount
             << "  hab " << countHabitableWorlds(i) << endl;
    }

    cout << endl;
}

void showAnomalyList(int systemIndex)
{
    if (systemIndex < 0 || systemIndex >= systemWorldCount)
        return;

    const systemworld &sys = systemWorldArray[systemIndex];

    cout << "    Objects:" << endl;

    for (int i = 0; i < sys.anomalyCount && i < 6; ++i)
    {
        const anomaly &a = sys.anomalies[i];

        cout << "      - " << a.name
             << "  (" << objectKindName(a.kind) << ")"
             << "  orbit " << a.orbitIndex
             << "  danger " << a.danger
             << "  reward " << a.reward << endl;
    }
}

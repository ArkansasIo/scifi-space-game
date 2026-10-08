// src/world/biome.c++ -- biomes, sub-biomes, world detail and the class tree.
// See docs/BIOMES.md.

#include "spacebattlerpg.h"
#include "world/biome.h"

/* ---------------- storage ---------------- */

biominfo biomeTable[BIOME_COUNT];
classinfo classTable[CLS_COUNT];

/* ---------------- biome seeding ---------------- */

// name, parent, minTemp, maxTemp, minWater, maxWater, habBonus,
// food, ore, energy, research, cost, hazard, unit
static const biominfo biomeSeeds[] = {
    {"None", BIOME_NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    /* rocky */
    {"Barren Rock", BIOME_NONE, 0, 400, 0, 10, -20, 0, 45, 20, 25, 100, 10, 1},
    {"Cratered", BIOME_BARREN_ROCK, 0, 350, 0, 5, -25, 0, 55, 10, 20, 110, 15, 1},
    {"Dust Flat", BIOME_BARREN_ROCK, 200, 450, 0, 15, -15, 5, 35, 30, 20, 95, 12, 1},

    /* hot */
    {"Volcanic", BIOME_NONE, 700, 2000, 0, 30, -35, 5, 60, 70, 30, 200, 75, 2},
    {"Lava Sea", BIOME_VOLCANIC, 900, 3000, 0, 20, -40, 0, 70, 90, 25, 260, 90, 2},
    {"Magma Cavern", BIOME_VOLCANIC, 600, 1500, 0, 10, -30, 0, 65, 80, 35, 220, 70, 2},

    /* cold */
    {"Ice Sheet", BIOME_NONE, 0, 180, 20, 80, -15, 10, 30, 25, 40, 130, 35, 1},
    {"Tundra", BIOME_NONE, 180, 260, 10, 50, 10, 25, 35, 20, 45, 110, 20, 1},
    {"Frozen Ocean", BIOME_ICE_SHEET, 0, 220, 70, 100, -10, 30, 25, 15, 55, 150, 30, 1},

    /* temperate */
    {"Grassland", BIOME_NONE, 250, 310, 20, 60, 25, 70, 25, 20, 50, 90, 5, 3},
    {"Forest", BIOME_NONE, 240, 300, 30, 70, 25, 55, 40, 15, 65, 100, 10, 3},
    {"Jungle", BIOME_FOREST, 280, 340, 50, 90, 15, 60, 45, 10, 80, 130, 30, 3},
    {"Savanna", BIOME_GRASSLAND, 300, 360, 10, 40, 15, 55, 30, 25, 45, 95, 12, 3},
    {"Wetland", BIOME_NONE, 260, 320, 60, 95, 15, 75, 20, 15, 60, 120, 18, 3},

    /* dry */
    {"Desert", BIOME_NONE, 310, 420, 0, 15, 0, 15, 35, 70, 35, 105, 25, 1},
    {"Dunes", BIOME_DESERT, 320, 450, 0, 10, -5, 10, 40, 60, 30, 115, 30, 1},
    {"Badlands", BIOME_DESERT, 300, 430, 0, 20, -10, 12, 50, 45, 30, 110, 28, 1},

    /* aquatic */
    {"Ocean", BIOME_NONE, 250, 320, 80, 100, 20, 65, 20, 25, 55, 140, 15, 3},
    {"Reef", BIOME_OCEAN, 260, 310, 70, 100, 20, 60, 25, 20, 75, 150, 12, 3},
    {"Archipelago", BIOME_OCEAN, 250, 320, 55, 85, 25, 70, 30, 20, 60, 130, 10, 3},

    /* exotic */
    {"Crystalline", BIOME_NONE, 100, 500, 0, 30, -20, 0, 75, 85, 90, 300, 45, 4},
    {"Fungal", BIOME_NONE, 200, 320, 30, 80, 5, 40, 35, 20, 85, 160, 35, 4},
    {"Gas Envelope", BIOME_NONE, 400, 1200, 0, 0, -40, 0, 20, 95, 60, 400, 60, 4},
    {"Crystal Cloud", BIOME_GAS_ENVELOPE, 200, 900, 0, 0, -35, 0, 30, 90, 95, 450, 55, 4},
    {"Hive World", BIOME_NONE, 250, 400, 20, 70, -10, 30, 50, 40, 100, 500, 95, 4},

    /* artificial */
    {"Ecumenopolis", BIOME_NONE, 200, 400, 0, 40, 20, 50, 30, 60, 100, 600, 20, 5},
    {"Machine World", BIOME_ECUMENOPOLIS, 150, 500, 0, 20, -15, 10, 80, 95, 110, 800, 70, 5},
    {"Ruined World", BIOME_NONE, 100, 500, 0, 60, -20, 15, 60, 40, 90, 250, 65, 4}};

static const int biomeSeedCount =
    (int)(sizeof(biomeSeeds) / sizeof(biomeSeeds[0]));

static const char *featureNames[FEATURE_COUNT] = {
    "None", "River", "Mountain Range", "Canyon", "Geothermal Vents",
    "Crystal Field", "Ruins", "Crater", "Ice Cavern", "Underwater Vent",
    "Floating Isles", "Subterranean Sea"};

void initializeBiomes()
{
    memset(biomeTable, 0, sizeof(biomeTable));

    for (int i = 0; i < biomeSeedCount && i < BIOME_COUNT; ++i)
        biomeTable[i] = biomeSeeds[i];
}

const biominfo *biomeInfo(biome b)
{
    if (b < 0 || b >= BIOME_COUNT)
        return &biomeTable[BIOME_NONE];

    return &biomeTable[b];
}

const char *biomeName(biome b)
{
    return biomeInfo(b)->name;
}

const char *biomeFeatureName(biomefeature f)
{
    if (f < 0 || f >= FEATURE_COUNT)
        return "None";

    return featureNames[f];
}

int biomeCount()
{
    return BIOME_COUNT;
}

/* ---------------- biome selection ---------------- */

biome selectBiome(int temperature, int waterCoverage, int habitability)
{
    // Hot bodies first: temperature dominates.
    if (temperature >= 900)
        return BIOME_LAVA_SEA;

    if (temperature >= 700)
        return BIOME_VOLCANIC;

    if (temperature >= 450)
    {
        if (waterCoverage > 50)
            return BIOME_GAS_ENVELOPE;

        return BIOME_DESERT;
    }

    // Cold bodies.
    if (temperature <= 120)
    {
        if (waterCoverage > 70)
            return BIOME_FROZEN_OCEAN;

        return BIOME_ICE_SHEET;
    }

    if (temperature <= 200)
        return BIOME_TUNDRA;

    // Temperate band: habitability and water decide.
    if (habitability >= 70)
    {
        if (waterCoverage > 70)
            return BIOME_ARCHIPELAGO;

        if (waterCoverage > 45)
            return BIOME_FOREST;

        return BIOME_GRASSLAND;
    }

    if (habitability >= 40)
    {
        if (waterCoverage > 75)
            return BIOME_OCEAN;

        if (waterCoverage > 40)
            return BIOME_WETLAND;

        return BIOME_SAVANNA;
    }

    if (habitability >= 15)
    {
        if (waterCoverage > 60)
            return BIOME_OCEAN;

        if (waterCoverage > 25)
            return BIOME_GRASSLAND;

        return BIOME_DUNES;
    }

    // Poor worlds.
    if (waterCoverage > 50)
        return BIOME_FROZEN_OCEAN;

    if (temperature > 350)
        return BIOME_BADLANDS;

    return BIOME_BARREN_ROCK;
}

biome selectSecondaryBiome(biome primary, int seed)
{
    const biominfo *info = biomeInfo(primary);

    // A child of the primary is the natural secondary.
    for (int i = 1; i < BIOME_COUNT; ++i)
    {
        if (biomeTable[i].parent == primary)
            return (biome)i;
    }

    // Otherwise pick a neighbouring band by temperature.
    int offset = ((seed % 3) - 1) * 2;

    if (offset == 0)
        offset = 2;

    int candidate = (int)primary + offset;

    if (candidate < 1)
        candidate = 1;
    if (candidate >= BIOME_COUNT)
        candidate = BIOME_COUNT - 1;

    (void)info;

    return (biome)candidate;
}

int rollBiomeFeatures(biomefeature out[], int outMax, int seed)
{
    if (!out || outMax <= 0)
        return 0;

    // Deterministic per-seed feature count: 0..3.
    int count = (seed / 7) % 4;

    if (count > outMax)
        count = outMax;

    for (int i = 0; i < count; ++i)
    {
        int pick = ((seed / (i + 2)) + i * 3) % FEATURE_COUNT;

        if (pick <= FEATURE_NONE)
            pick = FEATURE_RIVER;

        out[i] = (biomefeature)pick;
    }

    return count;
}

void buildWorldDetail(worlddetail &d, const struct world &w, int seed)
{
    memset(&d, 0, sizeof(d));

    d.primaryBiome = selectBiome(w.temperature, w.waterCoverage,
                                 w.habitability);

    d.secondaryBiome = selectSecondaryBiome(d.primaryBiome, seed);

    d.featureCount = rollBiomeFeatures(d.features, 4, seed);

    const biominfo *info = biomeInfo(d.primaryBiome);

    d.foodYield = info->foodYield;
    d.oreYield = info->oreYield + (w.resources / 2);
    d.energyYield = info->energyYield;
    d.researchYield = info->researchYield;

    // Terrain difficulty comes from gravity and the biome's hazard.
    d.terrainDifficulty = (w.gravity / 3) + (info->hazard / 2);

    if (d.terrainDifficulty > 100)
        d.terrainDifficulty = 100;

    d.hazardRating = info->hazard;

    // A hazardous world is harder to hold but yields more.
    if (d.hazardRating > 60)
        d.oreYield += 10;

    d.colonised = 0;
    d.population = 0;
}

int worldFoodYield(const worlddetail &d)
{
    return d.foodYield;
}

int worldOreYield(const worlddetail &d)
{
    return d.oreYield;
}

int worldEnergyYield(const worlddetail &d)
{
    return d.energyYield;
}

int worldResearchYield(const worlddetail &d)
{
    return d.researchYield;
}

int worldColoniseCost(const worlddetail &d)
{
    const biominfo *info = biomeInfo(d.primaryBiome);

    int cost = info->coloniseCost;

    // Terrain and hazards both raise the price.
    cost += d.terrainDifficulty * 2;
    cost += d.hazardRating * 3;

    return cost;
}

int worldIsSettleable(const worlddetail &d, int habitability)
{
    // Any world can be mined, but only decent ones can be settled.
    if (habitability < 25 && d.primaryBiome != BIOME_CRYSTALLINE
        && d.primaryBiome != BIOME_FUNGAL)
        return 0;

    if (d.hazardRating >= 95)
        return 0;   // hive worlds and machine worlds resist settlement

    return 1;
}

/* ---------------- class seeding ---------------- */

// name, parent, role, str dex con int wis, vit spi lck wil cha,
// hp/lvl, power/lvl, atk/lvl, def/lvl, resource, signature
static const classinfo classSeeds[] = {
    {"None", CLS_NONE, CLASSROLE_TANK, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
     RES_HP, ""},

    /* Warrior */
    {"Warrior", CLS_NONE, CLASSROLE_TANK, 12,6,10,3,4, 10,4,4,6,5,
     14, 6, 12, 10, RES_RAGE, "Shield Wall"},
    {"Guardian", CLS_WARRIOR, CLASSROLE_TANK, 14,5,14,3,6, 14,5,3,8,5,
     18, 5, 10, 14, RES_RAGE, "Bulwark"},
    {"Berserker", CLS_WARRIOR, CLASSROLE_DPS_MELEE, 16,7,8,3,3, 8,3,6,4,4,
     12, 6, 16, 8, RES_RAGE, "Bloodlust"},
    {"Champion", CLS_WARRIOR, CLASSROLE_DPS_MELEE, 15,8,9,4,4, 9,4,6,5,6,
     13, 6, 15, 9, RES_RAGE, "Executioner"},

    /* Scout */
    {"Scout", CLS_NONE, CLASSROLE_DPS_RANGED, 6,12,6,5,6, 7,6,8,5,5,
     10, 8, 13, 7, RES_ENERGY, "Mark Target"},
    {"Ranger", CLS_SCOUT, CLASSROLE_DPS_RANGED, 7,14,6,5,7, 7,7,9,6,5,
     10, 9, 14, 7, RES_FOCUS, "Volley"},
    {"Assassin", CLS_SCOUT, CLASSROLE_DPS_MELEE, 8,16,5,5,5, 6,6,10,6,4,
     9, 8, 18, 5, RES_ENERGY, "Shadow Strike"},
    {"Sniper", CLS_SCOUT, CLASSROLE_DPS_RANGED, 6,15,5,8,7, 6,7,11,5,4,
     9, 9, 16, 6, RES_FOCUS, "Railshot"},

    /* Engineer */
    {"Engineer", CLS_NONE, CLASSROLE_SUPPORT, 7,6,8,11,8, 9,9,5,6,6,
     11, 9, 9, 8, RES_HEAT, "Deploy Turret"},
    {"Technician", CLS_ENGINEER, CLASSROLE_SUPPORT, 7,7,8,12,9, 9,10,5,7,6,
     11, 10, 9, 9, RES_HEAT, "Overclock"},
    {"Mechanic", CLS_ENGINEER, CLASSROLE_SUPPORT, 9,6,10,11,8, 11,8,4,6,5,
     13, 8, 10, 11, RES_HEAT, "Field Repair"},
    {"Architect", CLS_ENGINEER, CLASSROLE_CONTROLLER, 6,6,8,14,10, 8,11,5,8,7,
     10, 11, 8, 8, RES_HEAT, "Fortify Zone"},

    /* Psionic */
    {"Psionic", CLS_NONE, CLASSROLE_DPS_MAGIC, 4,7,6,12,12, 7,12,6,10,8,
     9, 12, 14, 6, RES_AETHER, "Mind Spike"},
    {"Mystic", CLS_PSIONIC, CLASSROLE_HEALER, 4,6,7,12,16, 8,15,6,11,8,
     10, 14, 11, 7, RES_FAITH, "Soothing Light"},
    {"Telepath", CLS_PSIONIC, CLASSROLE_CONTROLLER, 3,8,6,15,13, 7,13,7,12,9,
     9, 13, 12, 6, RES_AETHER, "Dominate"},
    {"Warlock", CLS_PSIONIC, CLASSROLE_DPS_MAGIC, 5,7,6,16,10, 7,10,6,9,10,
     9, 12, 17, 5, RES_AETHER, "Void Lash"},

    /* Officer */
    {"Officer", CLS_NONE, CLASSROLE_SUPPORT, 8,8,8,8,8, 8,8,6,8,12,
     11, 8, 10, 9, RES_FOCUS, "Rally"},
    {"Commander", CLS_OFFICER, CLASSROLE_SUPPORT, 9,8,9,8,9, 9,9,6,9,15,
     12, 9, 11, 10, RES_FOCUS, "Battle Plan"},
    {"Admiral", CLS_OFFICER, CLASSROLE_SUPPORT, 9,8,10,9,10, 10,10,6,10,18,
     13, 10, 11, 11, RES_FOCUS, "Fleet Command"},
    {"Diplomat", CLS_OFFICER, CLASSROLE_HEALER, 6,7,7,10,12, 7,11,8,11,20,
     9, 11, 8, 7, RES_FAITH, "Concord"}};

static const int classSeedCount =
    (int)(sizeof(classSeeds) / sizeof(classSeeds[0]));

void initializeClasses()
{
    memset(classTable, 0, sizeof(classTable));

    for (int i = 0; i < classSeedCount && i < CLS_COUNT; ++i)
        classTable[i] = classSeeds[i];
}

const classinfo *classInfo(characterclass c)
{
    if (c < 0 || c >= CLS_COUNT)
        return &classTable[CLS_NONE];

    return &classTable[c];
}

const char *className(characterclass c)
{
    return classInfo(c)->name;
}

const char *classRoleName(classrole r)
{
    switch (r)
    {
    case CLASSROLE_TANK:
        return "Tank";
    case CLASSROLE_HEALER:
        return "Healer";
    case CLASSROLE_DPS_MELEE:
        return "Melee DPS";
    case CLASSROLE_DPS_RANGED:
        return "Ranged DPS";
    case CLASSROLE_DPS_MAGIC:
        return "Magic DPS";
    case CLASSROLE_SUPPORT:
        return "Support";
    case CLASSROLE_CONTROLLER:
        return "Controller";
    default:
        return "Unknown";
    }
}

int classCount()
{
    return CLS_COUNT;
}

characterclass findClass(const char *name)
{
    if (!name)
        return CLS_NONE;

    for (int i = 0; i < CLS_COUNT; ++i)
    {
        const char *candidate = classTable[i].name;

        if (!candidate)
            continue;

        if (strlen(candidate) != strlen(name))
            continue;

        int match = 1;

        for (int j = 0; name[j]; ++j)
        {
            if (tolower((unsigned char)candidate[j])
                != tolower((unsigned char)name[j]))
            {
                match = 0;
                break;
            }
        }

        if (match)
            return (characterclass)i;
    }

    return CLS_NONE;
}

int classChildren(characterclass parent, characterclass out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    int written = 0;

    for (int i = 1; i < CLS_COUNT && written < outMax; ++i)
    {
        if (classTable[i].parent == parent)
        {
            out[written] = (characterclass)i;
            written++;
        }
    }

    return written;
}

int classCanAdvance(characterclass from, characterclass to)
{
    if (from == to)
        return 0;

    if (to <= CLS_NONE || to >= CLS_COUNT)
        return 0;

    return (classTable[to].parent == from) ? 1 : 0;
}

classrole classRoleOf(characterclass c)
{
    return classInfo(c)->role;
}

void applyClass(struct statblock &stats, characterclass c)
{
    const classinfo *info = classInfo(c);

    // Baseline attributes from the class, then re-derive.
    stats.attributes.str += info->baseStr;
    stats.attributes.dex += info->baseDex;
    stats.attributes.con += info->baseCon;
    stats.attributes.intel += info->baseInt;
    stats.attributes.wis += info->baseWis;
    stats.attributes.vit += info->baseVit;
    stats.attributes.spi += info->baseSpi;
    stats.attributes.lck += info->baseLck;
    stats.attributes.wil += info->baseWil;
    stats.attributes.cha += info->baseCha;

    deriveSubAttributes(stats);
    deriveCombatStats(stats);
}

/* ---------------- presentation ---------------- */

void showBiomeList()
{
    drawHeader("Biomes");

    for (int i = 1; i < BIOME_COUNT; ++i)
    {
        const biominfo &b = biomeTable[i];

        cout << "  " << i << ". " << b.name;

        int length = (int)strlen(b.name);
        for (int pad = length; pad < 18; ++pad)
            cout << ' ';

        cout << " food " << b.foodYield
             << "  ore " << b.oreYield
             << "  energy " << b.energyYield
             << "  res " << b.researchYield << endl;
    }

    cout << endl;
}

void showBiomeDetail(biome b)
{
    const biominfo *info = biomeInfo(b);

    drawHeader(info->name);

    drawFieldText("Parent", biomeName(info->parent));
    drawField("Temperature band", info->minTemperature);
    drawField("to", info->maxTemperature);
    drawField("Water min", info->minWater);
    drawField("Water max", info->maxWater);
    drawField("Habitability bonus", info->habitabilityBonus);
    drawField("Food yield", info->foodYield);
    drawField("Ore yield", info->oreYield);
    drawField("Energy yield", info->energyYield);
    drawField("Research yield", info->researchYield);
    drawField("Colonise cost", info->coloniseCost);
    drawField("Hazard", info->hazard);

    // List its sub-biomes.
    cout << endl << "  Sub-biomes:" << endl;

    int found = 0;

    for (int i = 1; i < BIOME_COUNT; ++i)
    {
        if (biomeTable[i].parent == b)
        {
            cout << "    - " << biomeTable[i].name << endl;
            found++;
        }
    }

    if (found == 0)
        out("(none)");

    cout << endl;
}

void showClassTree()
{
    drawHeader("Classes");

    // Base classes first, then their children indented.
    for (int i = 1; i < CLS_COUNT; ++i)
    {
        if (classTable[i].parent != CLS_NONE)
            continue;

        cout << "  " << classTable[i].name
             << "  (" << classRoleName(classTable[i].role) << ")" << endl;

        for (int j = 1; j < CLS_COUNT; ++j)
        {
            if (classTable[j].parent != (characterclass)i)
                continue;

            cout << "      - " << classTable[j].name
                 << "  (" << classRoleName(classTable[j].role) << ")" << endl;
        }
    }

    cout << endl;
}

void showClassDetail(characterclass c)
{
    const classinfo *info = classInfo(c);

    drawHeader(info->name);

    drawFieldText("Role", classRoleName(info->role));
    drawFieldText("Advances from", className(info->parent));
    drawFieldText("Signature", info->signatureSkill);
    drawFieldText("Resource", resourceName((resourcetype)info->resourceType));

    cout << endl << "  Base attributes:" << endl;
    drawField("STR", info->baseStr);
    drawField("DEX", info->baseDex);
    drawField("CON", info->baseCon);
    drawField("INT", info->baseInt);
    drawField("WIS", info->baseWis);

    cout << endl << "  Per level:" << endl;
    drawField("HP", info->hpPerLevel);
    drawField("Power", info->powerPerLevel);
    drawField("Attack", info->attackPerLevel);
    drawField("Defence", info->defencePerLevel);

    cout << endl;
}

void showWorldDetail(const worlddetail &d)
{
    drawHeader("World Detail");

    drawFieldText("Primary biome", biomeName(d.primaryBiome));
    drawFieldText("Secondary biome", biomeName(d.secondaryBiome));

    cout << endl << "  Yields:" << endl;
    drawField("Food", d.foodYield);
    drawField("Ore", d.oreYield);
    drawField("Energy", d.energyYield);
    drawField("Research", d.researchYield);

    cout << endl;
    drawField("Terrain difficulty", d.terrainDifficulty);
    drawField("Hazard rating", d.hazardRating);
    drawField("Colonise cost", worldColoniseCost(d));
    drawField("Colonised", d.colonised);

    if (d.featureCount > 0)
    {
        cout << endl << "  Features:" << endl;

        for (int i = 0; i < d.featureCount && i < 4; ++i)
            cout << "    - " << biomeFeatureName(d.features[i]) << endl;
    }

    cout << endl;
}

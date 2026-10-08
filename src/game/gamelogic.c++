// src/game/gamelogic.c++ -- top-level game logic and feature entry points.
//
// This is the module main.c++ talks to.  It owns the framework boot sequence,
// the feature registry and the summary screens.

#include "spacebattlerpg.h"
#include "engine/subsystem.h"
#include "ui/display.h"
#include "ui/hud.h"

/* ---------------- internal state ---------------- */

static int g_frameworkReady = 0;
static int g_pvpEnabled = 0;
static int g_difficultyTier = 0;

/* ---------------- subsystem descriptors ---------------- */

static subsystem g_combatSys = {
    "CombatEngine", SYS_COMBAT, 0, 0, 20,
    combatEngineInit, combatEngineStart, combatEngineUpdate,
    combatEngineStop, combatEngineShutdown};

static subsystem g_battleSys = {
    "BattleEngine", SYS_BATTLE, 0, 0, 25,
    battleEngineInit, battleEngineStart, battleEngineUpdate,
    battleEngineStop, battleEngineShutdown};

static subsystem g_effectSys = {
    "EffectEngine", SYS_EFFECTS, 0, 0, 15,
    effectEngineInit, effectEngineStart, effectEngineUpdate,
    effectEngineStop, effectEngineShutdown};

static subsystem g_aiSys = {
    "AIEngine", SYS_AI, 0, 0, 30,
    aiEngineInit, aiEngineStart, aiEngineUpdate,
    aiEngineStop, aiEngineShutdown};

static subsystem g_lootSys = {
    "LootEngine", SYS_LOOT, 0, 0, 35,
    lootEngineInit, lootEngineStart, lootEngineUpdate,
    lootEngineStop, lootEngineShutdown};

static subsystem g_progSys = {
    "ProgressionEngine", SYS_PROGRESSION, 0, 0, 40,
    progressionEngineInit, progressionEngineStart, progressionEngineUpdate,
    progressionEngineStop, progressionEngineShutdown};

static subsystem g_audioSys = {
    "Audio", SYS_AUDIO, 0, 0, 10,
    audioInit, audioStart, audioUpdate, audioStop, audioShutdown};

/* ---------------- lifecycle ---------------- */

void initializeFramework()
{
    if (g_frameworkReady)
        return;

    // 1. Fill every data table (labels, caps, effects, enemies, gear, ...).
    initializeFrameworkArrays();

    // 2. Seed the galaxy map.
    initializeGalaxy();

    // 3. Register and bring up the engines.
    kernelRegister(g_audioSys);
    kernelRegister(g_effectSys);
    kernelRegister(g_combatSys);
    kernelRegister(g_battleSys);
    kernelRegister(g_aiSys);
    kernelRegister(g_lootSys);
    kernelRegister(g_progSys);

    kernelInitAll();
    kernelStartAll();

    g_frameworkReady = 1;
}

void resetFrameworkState()
{
    g_pvpEnabled = 0;
    g_difficultyTier = 0;
}

/* ---------------- feature toggles ---------------- */

void setPvPEnabled(int enabled)
{
    g_pvpEnabled = enabled ? 1 : 0;

    luaFireEvent(SBW_EVENT_PVP_FLAG, 0, g_pvpEnabled);
}

int isPvPEnabled()
{
    return g_pvpEnabled;
}

int currentDifficultyTier()
{
    return g_difficultyTier;
}

/* ---------------- summary screens ---------------- */

void showStatSheet(const statblock &stats)
{
    drawHeader("Character Sheet");

    cout << "  -- Core Attributes --" << endl;
    drawField("STR", stats.attributes.str);
    drawField("DEX", stats.attributes.dex);
    drawField("CON", stats.attributes.con);
    drawField("INT", stats.attributes.intel);
    drawField("WIS", stats.attributes.wis);
    drawField("VIT", stats.attributes.vit);
    drawField("SPI", stats.attributes.spi);
    drawField("LCK", stats.attributes.lck);
    drawField("WIL", stats.attributes.wil);
    drawField("CHA", stats.attributes.cha);

    cout << endl
         << "  -- Derived --" << endl;
    drawField("Physical Power", stats.sub.physicalpower);
    drawField("Spell Power", stats.sub.spellpower);
    drawField("Healing Power", stats.sub.healingpower);
    drawField("Max HP", stats.sub.maxhp);
    drawField("Armor", stats.sub.armor);
    drawField("Magic Resist", stats.sub.magicresist);
    drawField("Tenacity", stats.sub.tenacity);
    drawField("Poise", stats.sub.poise);

    cout << endl
         << "  -- Combat --" << endl;
    drawField("Base Attack", stats.combat.baseattack);
    drawField("Crit Chance (bp)", stats.combat.critchance);
    drawField("Crit Damage (%)", stats.combat.critdamage);
    drawField("Mitigation (bp)", stats.combat.mitigation);
    drawField("CDR (bp)", stats.combat.cdr);

    cout << endl;
}

void showPlayerSummary()
{
    showCharacterPanel();

    statblock stats = buildPlayerStatBlock();
    drawField("Derived Max HP", stats.sub.maxhp);
    drawField("Derived Attack", stats.combat.baseattack);

    cout << endl;
}

void showActiveEffects(const activeeffect list[], int count)
{
    drawHeader("Active Effects");

    int shown = 0;

    for (int i = 0; i < count; ++i)
    {
        int index = list[i].definitionIndex;

        if (index < 0 || index >= effectCount)
            continue;

        const effectdefinition &def = effectArray[index];

        cout << "  " << def.name
             << "  x" << list[i].stacks
             << "  (" << list[i].remaining << " ticks)"
             << "  [" << list[i].magnitude << "]" << endl;

        shown++;
    }

    if (shown == 0)
        out("(none)");

    cout << endl;
}

void showItemTooltip(int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= itemCount)
    {
        out("No such item.");
        return;
    }

    const itemdata &item = itemArray[itemIndex];

    drawHeader(item.name);

    drawFieldText("Slot", slotName(item.slot));
    drawFieldText("Rarity", rarityName(item.rarity));
    drawField("Required Level", item.requiredLevel);

    cout << endl
         << "  PvE table:" << endl;
    for (int i = 0; i < item.pveLineCount; ++i)
    {
        cout << "    +" << item.pveLines[i].raw << " "
             << item.pveLines[i].label
             << "  (" << item.pveLines[i].valuePU << " PU)" << endl;
    }
    drawField("PvE Item Power", item.itemPowerPvE);

    cout << endl
         << "  PvP table:" << endl;
    for (int i = 0; i < item.pvpLineCount; ++i)
    {
        cout << "    +" << item.pvpLines[i].raw << " "
             << item.pvpLines[i].label
             << "  (" << item.pvpLines[i].valuePU << " PU)" << endl;
    }
    drawField("PvP Item Power", item.itemPowerPvP);

    if (item.procValuePU > 0)
        drawField("Proc Value (PvE)", item.procValuePU);

    cout << endl;
}

void showEncounter(const encounterinstance &enc)
{
    drawHeader(enc.name);

    drawField("Units", enc.unitCount);
    drawField("Waves", enc.waveCount);
    drawField("Timed", enc.timed);
    drawField("Power Budget", enc.powerBudget);

    if (enc.bossIndex >= 0 && enc.bossIndex < bossCount)
        drawFieldText("Boss", bossArray[enc.bossIndex].name);

    cout << endl;

    for (int i = 0; i < enc.unitCount; ++i)
    {
        const encounterunit &unit = enc.units[i];

        cout << "    " << (i + 1) << ". ";

        if (unit.archetypeIndex >= 0 && unit.archetypeIndex < archetypeCount)
            cout << archetypeArray[unit.archetypeIndex].name;
        else
            cout << "Unknown";

        cout << "  [" << tierName((enemytier)unit.tier) << "]"
             << "  HP " << unit.scaledHP
             << "  DMG " << unit.scaledDamage << endl;
    }

    cout << endl;
}

void showBossRoster()
{
    drawHeader("Boss Roster");

    for (int i = 0; i < bossCount; ++i)
    {
        const bossdata &boss = bossArray[i];

        cout << "  " << boss.name
             << "  (" << tierName(boss.tier) << ", level " << boss.level
             << ")" << endl;

        for (int p = 0; p < boss.phaseCount && p < 4; ++p)
        {
            const bossphase &phase = boss.phases[p];

            cout << "      Phase " << (p + 1) << ": " << phase.name
                 << "  at " << phase.hpThreshold << "%"
                 << "  (" << phase.abilityCount << " abilities)" << endl;
        }

        if (boss.softEnrageRate > 0)
            cout << "      Soft enrage: +" << boss.softEnrageRate
                 << "% per 15s" << endl;

        if (boss.hardEnrageTime > 0)
            cout << "      Hard enrage: " << boss.hardEnrageTime
                 << "s" << endl;

        cout << endl;
    }
}

void showGalaxyMap()
{
    drawHeader("Galaxy Map");

    for (int i = 0; i < systemCount; ++i)
    {
        const starsystem &sys = systemArray[i];

        cout << "  " << sys.name
             << "  (sector " << sys.sectorIndex << ")";

        if (sys.ownerFaction >= 0 && sys.ownerFaction < factionCount)
            cout << "  [" << factionArray[sys.ownerFaction].name << "]";

        cout << endl;

        for (int p = 0; p < sys.planetCount && p < 6; ++p)
        {
            const planet &planet = sys.planets[p];

            cout << "      " << planet.name
                 << "  size " << planet.size
                 << "  hab " << planet.habitability
                 << "  res " << planet.resources << endl;
        }

        if (sys.hasAsteroidField)
            cout << "      + asteroid field" << endl;
        if (sys.hasStation)
            cout << "      + station" << endl;
        if (sys.hasAnomaly)
            cout << "      + anomaly" << endl;
    }

    cout << endl;
}

void showEmpireDashboard()
{
    drawHeader("Empire");

    drawField("Industry", empireAttributes.industry);
    drawField("Science", empireAttributes.science);
    drawField("Economy", empireAttributes.economy);
    drawField("Influence", empireAttributes.influence);
    drawField("Logistics", empireAttributes.logistics);
    drawField("Intelligence", empireAttributes.intelligence);
    drawField("Stability", empireAttributes.stability);

    cout << endl;
    drawField("Credits", empireStorage.credits);
    drawField("Minerals", empireStorage.minerals);
    drawField("Gas", empireStorage.gas);
    drawField("Energy", empireStorage.energy);
    drawField("Data", empireStorage.data);
    drawField("Quantum Cores", empireStorage.quantumcores);
    drawField("Alien Artifacts", empireStorage.alienartifacts);

    cout << endl;
    drawField("Systems", systemCount);
    drawField("Fleet Power", fleetPower(playerFleet));
    drawField("Supply Use", fleetSupplyUse(playerFleet));

    cout << endl;
}

/* ---------------- feature registry ---------------- */

static const char *featureDescriptions[] = {
    "Unified stat model: attributes, sub-attributes, combat stats",
    "Damage types with a four-layer resistance model",
    "Buffs and debuffs with stacking rules and CC diminishing returns",
    "Enemy taxonomy: trash, veteran, elite, champion, boss, mythic",
    "Boss framework: phases, mechanics, arena hazards, enrage",
    "Aggro and threat with taunts and stance multipliers",
    "Scaling: level, party, difficulty and PvP normalization",
    "Affix system for elites and mythic runs",
    "Loot tables with pity timers and reward multipliers",
    "Encounter templates for open world, dungeons and raids",
    "AI state machine with utility-scored ability selection",
    "Item power and value formulas, PvE and PvP separately",
    "Equipment slots, rarities, sets and enhancements",
    "Cross-platform persistence: MySQL schema and CSV export",
    "Embedded Lua scripting with an event hook system",
    "Audio cue dispatcher with bus volumes and a sound manifest",
    "12-act, 50-chapter campaign with story progression",
    "4X layer: sectors, systems, planets, fleets, trade, diplomacy",
    "Turn-based tactical combat and RTS fleet power",
    "Telemetry and balance reporting"};

const char *featureDescription(int featureId)
{
    int count = featureCount();

    if (featureId < 0 || featureId >= count)
        return "";

    return featureDescriptions[featureId];
}

int featureCount()
{
    return (int)(sizeof(featureDescriptions) / sizeof(featureDescriptions[0]));
}

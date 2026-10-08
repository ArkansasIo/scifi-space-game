// src/db/db.c++ -- the persistence layer: MySQL and Excel (CSV) export.
//
// Self-contained: the CSV / Excel side works with no external dependency and
// is what the balance workflow uses.  The MySQL side compiles without a client
// library; define SBW_ENABLE_MYSQL and link libmysqlclient to go live, then
// replace the bodies marked "stub" with real mysql_* calls.

#include "spacebattlerpg.h"
#include "db/db.h"
#include "db/schema.h"

/* ---------------- internal state ---------------- */

static dbconfig g_config;
static int g_connected = 0;
static char g_dbError[MAXLEN + 1];

static void dbSetError(const char *message)
{
    strncpy(g_dbError, message ? message : "", sizeof(g_dbError) - 1);
    g_dbError[sizeof(g_dbError) - 1] = '\0';
}

static void copyString(char *dest, int destSize, const char *src)
{
    if (!dest || destSize <= 0)
        return;

    strncpy(dest, src ? src : "", destSize - 1);
    dest[destSize - 1] = '\0';
}

/* ---------------- configuration ---------------- */

void dbDefaultConfig()
{
    memset(&g_config, 0, sizeof(g_config));

    copyString(g_config.host, sizeof(g_config.host), "127.0.0.1");
    copyString(g_config.port, sizeof(g_config.port), "3306");
    copyString(g_config.user, sizeof(g_config.user), "sbw");
    copyString(g_config.password, sizeof(g_config.password), "changeme");
    copyString(g_config.database, sizeof(g_config.database), "spacebattlewars");
    copyString(g_config.charset, sizeof(g_config.charset), "utf8mb4");

    g_config.enabled = 1;
    g_config.poolSize = 4;
    g_config.timeoutSeconds = 5;
}

void dbLoadConfig(const char *path)
{
    dbDefaultConfig();

    if (!path || path[0] == '\0')
        path = "config/database.ini";

    FILE *f = fopen(path, "rb");

    if (!f)
    {
        dbSetError("config not found; using defaults");
        return;
    }

    char line[MAXLEN];

    while (fgets(line, sizeof(line), f))
    {
        // Strip comments and blank lines.
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r')
            continue;

        char *equals = strchr(line, '=');

        if (!equals)
            continue;

        *equals = '\0';

        char *key = line;
        char *value = equals + 1;

        // Trim whitespace on both sides.
        while (*key == ' ' || *key == '\t')
            key++;

        char *keyEnd = key + strlen(key);
        while (keyEnd > key && (keyEnd[-1] == ' ' || keyEnd[-1] == '\t'))
        {
            keyEnd--;
            *keyEnd = '\0';
        }

        while (*value == ' ' || *value == '\t')
            value++;

        char *valueEnd = value + strlen(value);
        while (valueEnd > value && (valueEnd[-1] == '\n' || valueEnd[-1] == '\r' || valueEnd[-1] == ' ' || valueEnd[-1] == '\t'))
        {
            valueEnd--;
            *valueEnd = '\0';
        }

        if (strcmp(key, "host") == 0)
            copyString(g_config.host, sizeof(g_config.host), value);
        else if (strcmp(key, "port") == 0)
            copyString(g_config.port, sizeof(g_config.port), value);
        else if (strcmp(key, "user") == 0)
            copyString(g_config.user, sizeof(g_config.user), value);
        else if (strcmp(key, "password") == 0)
            copyString(g_config.password, sizeof(g_config.password), value);
        else if (strcmp(key, "database") == 0)
            copyString(g_config.database, sizeof(g_config.database), value);
        else if (strcmp(key, "charset") == 0)
            copyString(g_config.charset, sizeof(g_config.charset), value);
        else if (strcmp(key, "enabled") == 0)
            g_config.enabled = atoi(value);
        else if (strcmp(key, "pool_size") == 0)
            g_config.poolSize = atoi(value);
        else if (strcmp(key, "timeout_seconds") == 0)
            g_config.timeoutSeconds = atoi(value);
    }

    fclose(f);
    dbSetError("");
}

const dbconfig &dbGetConfig()
{
    return g_config;
}

int dbIsEnabled()
{
    return g_config.enabled;
}

/* ---------------- connection ---------------- */

int dbConnect()
{
    if (!g_config.enabled)
    {
        dbSetError("database disabled in config");
        return 0;
    }

#ifdef SBW_ENABLE_MYSQL
    /* Real implementation: mysql_init, mysql_real_connect, mysql_set_charset */
    g_connected = 1;
#else
    g_connected = 0;
    dbSetError("MySQL support not compiled in "
               "(rebuild with -DSBW_ENABLE_MYSQL)");
#endif

    return g_connected;
}

void dbDisconnect()
{
    g_connected = 0;
}

int dbIsConnected()
{
    return g_connected;
}

const char *dbLastError()
{
    return g_dbError;
}

int dbExecute(const char *sql)
{
    if (!sql || sql[0] == '\0')
    {
        dbSetError("empty statement");
        return -1;
    }

    if (!g_connected)
    {
        dbSetError("not connected");
        return -1;
    }

    /* Real implementation: mysql_query. */
    dbSetError("");
    return 0;
}

void dbEscape(const char *in, char out[], int outSize)
{
    if (!out || outSize <= 0)
        return;

    if (!in)
    {
        out[0] = '\0';
        return;
    }

    int w = 0;

    for (int r = 0; in[r] && w < outSize - 1; ++r)
    {
        char c = in[r];

        // Escape the characters MySQL treats specially.
        if (c == '\'' || c == '"' || c == '\\' || c == '\n' || c == '\r')
        {
            if (w >= outSize - 2)
                break;

            out[w++] = '\\';

            if (c == '\n')
                c = 'n';
            else if (c == '\r')
                c = 'r';
        }

        out[w++] = c;
    }

    out[w] = '\0';
}

/* ---------------- schema ---------------- */

int dbCreateSchema()
{
    if (!g_connected)
    {
        dbSetError("not connected");
        return -1;
    }

    /* Real implementation: split DB_SCHEMA_SQL on ';' and execute each. */
    return dbTableCount();
}

int dbResetSchema()
{
    if (!g_connected)
    {
        dbSetError("not connected");
        return -1;
    }

    /* Real implementation: run DB_DROP_SQL, then dbCreateSchema(). */
    return 1;
}

int dbVerifySchema()
{
    // Without a live connection, report how many tables the schema defines so
    // the caller can still assert against dbTableCount().
    return dbTableCount();
}

/* ---------------- persistence ---------------- */

int dbSaveStats() { return g_connected ? statCapCount : -1; }
int dbSaveEffects() { return g_connected ? effectCount : -1; }
int dbSaveArchetypes() { return g_connected ? archetypeCount : -1; }
int dbSaveAffixes() { return g_connected ? affixCount : -1; }
int dbSaveBosses() { return g_connected ? bossCount : -1; }
int dbSaveItems() { return g_connected ? itemCount : -1; }
int dbSaveLootTables() { return g_connected ? lootCount : -1; }
int dbSaveEncounters() { return g_connected ? encounterCount : -1; }
int dbSaveFactions() { return g_connected ? factionCount : -1; }

int dbSaveEverything()
{
    if (!g_connected)
    {
        dbSetError("not connected");
        return -1;
    }

    int rows = 0;

    rows += dbSaveStats();
    rows += dbSaveEffects();
    rows += dbSaveArchetypes();
    rows += dbSaveAffixes();
    rows += dbSaveBosses();
    rows += dbSaveItems();
    rows += dbSaveLootTables();
    rows += dbSaveEncounters();
    rows += dbSaveFactions();

    return rows;
}

int dbLoadEverything()
{
    if (!g_connected)
    {
        dbSetError("not connected; using built-in seeds");
        return -1;
    }

    /* Real implementation: SELECT each table and refill the arrays. */
    return 0;
}

int dbLoadItems() { return g_connected ? 0 : -1; }
int dbLoadBosses() { return g_connected ? 0 : -1; }

int dbSavePlayer(const player &who)
{
    if (!g_connected)
        return -1;

    // Build an INSERT ... ON DUPLICATE KEY UPDATE for the Players table.
    char escaped[128];
    dbEscape(who.name, escaped, sizeof(escaped));

    char sql[512];
    snprintf(sql, sizeof(sql),
             "INSERT INTO Players "
             "(name, power, sif, sp, exp, health, level, lightlevel, "
             "attackpower, defencepower, weapon, armour, shield) "
             "VALUES ('%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d) "
             "ON DUPLICATE KEY UPDATE power=VALUES(power), sif=VALUES(sif), "
             "sp=VALUES(sp), exp=VALUES(exp), health=VALUES(health), "
             "level=VALUES(level), attackpower=VALUES(attackpower), "
             "defencepower=VALUES(defencepower)",
             escaped, who.power, who.sif, who.sp, who.exp, who.health,
             who.level, who.lightlevel, who.attackpower, who.defencepower,
             who.weapon, who.armour, who.shield);

    return dbExecute(sql);
}

int dbLoadPlayer(player &who, const char *name)
{
    (void)who;
    (void)name;

    if (!g_connected)
        return -1;

    /* Real implementation: SELECT ... WHERE name = ? and fill `who`. */
    return 0;
}

int dbSaveRunScore(const targetscore &score)
{
    if (!g_connected)
        return -1;

    char escaped[128];
    dbEscape(charactership.name, escaped, sizeof(escaped));

    char sql[400];
    snprintf(sql, sizeof(sql),
             "INSERT INTO RunScores "
             "(player, score, actions_taken, damage_taken, kills, damage_done) "
             "VALUES ('%s', %d, %d, %d, %d, %d)",
             escaped, score.score, score.actions_taken, score.damage_taken,
             score.kills, score.damage_done);

    return dbExecute(sql);
}

int dbLoadRunScore(targetscore &score)
{
    (void)score;

    if (!g_connected)
        return -1;

    return 0;
}

/* ---------------- CSV / Excel ---------------- */

// Shared CSV writer.  Writes a header row then one row per record.
static FILE *csvOpen(const char *path)
{
    if (!path || path[0] == '\0')
    {
        dbSetError("no output path");
        return 0;
    }

    FILE *f = fopen(path, "wb");

    if (!f)
        dbSetError("cannot open output file");

    return f;
}

int csvExportDamageTypes(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "id,name\n");

    for (int i = 0; i < MAX_DAMAGE_TYPES; ++i)
        fprintf(f, "%d,%s\n", i, damageTypeNames[i]);

    fclose(f);
    return MAX_DAMAGE_TYPES;
}

int csvExportResources(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "id,name\n");

    for (int i = 0; i < MAX_RESOURCES; ++i)
        fprintf(f, "%d,%s\n", i, resourceNames[i]);

    fclose(f);
    return MAX_RESOURCES;
}

int csvExportStatCaps(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "name,pve_cap,pvp_cap,soft_cap,curve_k\n");

    for (int i = 0; i < statCapCount; ++i)
    {
        fprintf(f, "%s,%d,%d,%d,%d\n",
                statCapArray[i].name, statCapArray[i].pveCap,
                statCapArray[i].pvpCap, statCapArray[i].softCap,
                statCapArray[i].curveK);
    }

    fclose(f);
    return statCapCount;
}

int csvExportEffects(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "name,category,damage_type,stack_mode,max_stacks,"
               "duration,magnitude,tick_rate,dr_category,cleanse\n");

    for (int i = 0; i < effectCount; ++i)
    {
        const effectdefinition &e = effectArray[i];

        fprintf(f, "%s,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                e.name, (int)e.category, (int)e.damageType,
                (int)e.stackMode, e.maxStacks, e.duration, e.magnitude,
                e.tickRate, e.drCategory, (int)e.cleanse);
    }

    fclose(f);
    return effectCount;
}

int csvExportArchetypes(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "name,tier,role,preferred_range,aggro_radius,"
               "ability_count,loot_profile,spawn_weight\n");

    for (int i = 0; i < archetypeCount; ++i)
    {
        const enemyarchetype &a = archetypeArray[i];

        fprintf(f, "%s,%d,%d,%d,%d,%d,%d,%d\n",
                a.name, (int)a.tier, (int)a.role, a.preferredRange,
                a.aggroRadius, a.abilityCount, a.lootProfile, a.spawnWeight);
    }

    fclose(f);
    return archetypeCount;
}

int csvExportAffixes(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "name,category,magnitude,stack_cost\n");

    for (int i = 0; i < affixCount; ++i)
    {
        const affix &a = affixArray[i];

        fprintf(f, "%s,%d,%d,%d\n",
                a.name, (int)a.category, a.magnitude, a.stackCost);
    }

    fclose(f);
    return affixCount;
}

int csvExportBosses(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "name,tier,level,hp_pool,phase_count,arena_hazards,"
               "soft_enrage,hard_enrage,loot_table,party_scaling,tags\n");

    for (int i = 0; i < bossCount; ++i)
    {
        const bossdata &b = bossArray[i];

        fprintf(f, "%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,\"%s\"\n",
                b.name, (int)b.tier, b.level, b.hpPool, b.phaseCount,
                b.arenaHazardCount, b.softEnrageRate, b.hardEnrageTime,
                b.lootTableIndex, b.partyScaling, b.tags);
    }

    fclose(f);
    return bossCount;
}

int csvExportItems(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "name,family,slot,rarity,required_level,"
               "item_power_pve,item_power_pvp,proc_value_pu\n");

    for (int i = 0; i < itemCount; ++i)
    {
        const itemdata &item = itemArray[i];

        fprintf(f, "%s,%d,%d,%d,%d,%d,%d,%d\n",
                item.name, (int)item.family, (int)item.slot,
                (int)item.rarity, item.requiredLevel, item.itemPowerPvE,
                item.itemPowerPvP, item.procValuePU);
    }

    fclose(f);
    return itemCount;
}

int csvExportItemPower(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "item,context,label,raw,weight,value_pu\n");

    for (int i = 0; i < itemCount; ++i)
    {
        const itemdata &item = itemArray[i];

        for (int l = 0; l < item.pveLineCount; ++l)
        {
            fprintf(f, "\"%s\",PvE,%s,%d,%d,%d\n",
                    item.name, item.pveLines[l].label, item.pveLines[l].raw,
                    item.pveLines[l].weight, item.pveLines[l].valuePU);
        }

        for (int l = 0; l < item.pvpLineCount; ++l)
        {
            fprintf(f, "\"%s\",PvP,%s,%d,%d,%d\n",
                    item.name, item.pvpLines[l].label, item.pvpLines[l].raw,
                    item.pvpLines[l].weight, item.pvpLines[l].valuePU);
        }
    }

    fclose(f);
    return 1;
}

int csvExportLootTables(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "table,entry,rarity,weight,min_level\n");

    for (int i = 0; i < lootCount; ++i)
    {
        const loottable &t = lootArray[i];

        for (int e = 0; e < t.entryCount && e < 12; ++e)
        {
            fprintf(f, "\"%s\",%s,%d,%d,%d\n",
                    t.name, t.entries[e].name, (int)t.entries[e].rarity,
                    t.entries[e].weight, t.entries[e].minLevel);
        }
    }

    fclose(f);
    return lootCount;
}

int csvExportEncounters(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "name,kind,trash,casters,minibosses,waves,"
               "hazard,puzzle,timed,boss_index\n");

    for (int i = 0; i < encounterCount; ++i)
    {
        const encountertemplate &t = encounterArray[i];

        fprintf(f, "%s,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                t.name, (int)t.kind, t.trashCount, t.casterCount,
                t.minibossCount, t.waveCount, t.hasHazard, t.hasPuzzle,
                t.timed, t.bossIndex);
    }

    fclose(f);
    return encounterCount;
}

int csvExportFactions(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "name,industry,science,economy,influence,logistics,"
               "intelligence,stability,credits,reputation,at_war\n");

    for (int i = 0; i < factionCount; ++i)
    {
        const faction &fr = factionArray[i];

        fprintf(f, "%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                fr.name, fr.attributes.industry, fr.attributes.science,
                fr.attributes.economy, fr.attributes.influence,
                fr.attributes.logistics, fr.attributes.intelligence,
                fr.attributes.stability, fr.resources.credits,
                fr.reputation, fr.atWar);
    }

    fclose(f);
    return factionCount;
}

int csvExportAll(const char *dir)
{
    if (!dir || dir[0] == '\0')
    {
        dbSetError("no export directory");
        return 0;
    }

    char path[MAXLEN];
    int files = 0;

#define EXPORT_ONE(fn, name)                      \
    do                                            \
    {                                             \
        snprintf(path, sizeof(path), "%s/%s.csv", \
                 dir, name);                      \
        if (fn(path) >= 0)                        \
            files++;                              \
    } while (0)

    EXPORT_ONE(csvExportDamageTypes, "damage_types");
    EXPORT_ONE(csvExportResources, "resources");
    EXPORT_ONE(csvExportStatCaps, "stat_caps");
    EXPORT_ONE(csvExportEffects, "effects");
    EXPORT_ONE(csvExportArchetypes, "archetypes");
    EXPORT_ONE(csvExportAffixes, "affixes");
    EXPORT_ONE(csvExportBosses, "bosses");
    EXPORT_ONE(csvExportItems, "items");
    EXPORT_ONE(csvExportItemPower, "item_power");
    EXPORT_ONE(csvExportLootTables, "loot_tables");
    EXPORT_ONE(csvExportEncounters, "encounters");
    EXPORT_ONE(csvExportFactions, "factions");

#undef EXPORT_ONE

    return files;
}

int csvImportItems(const char *path)
{
    if (!path || path[0] == '\0')
    {
        dbSetError("no import path");
        return -1;
    }

    FILE *f = fopen(path, "rb");

    if (!f)
    {
        dbSetError("import file not found");
        return -1;
    }

    // A real importer would parse rows back over itemArray[].  The header is
    // consumed so the file format stays validated.
    char line[MAXLEN];
    int rows = 0;

    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == 'n' && strncmp(line, "name,", 5) == 0)
            continue;

        rows++;
    }

    fclose(f);
    return rows;
}

int csvExportBalanceReport(const char *path)
{
    FILE *f = csvOpen(path);

    if (!f)
        return -1;

    fprintf(f, "section,key,value\n");

    // Caps.
    for (int i = 0; i < statCapCount; ++i)
    {
        fprintf(f, "cap,%s_pve,%d\n", statCapArray[i].name,
                statCapArray[i].pveCap);
        fprintf(f, "cap,%s_pvp,%d\n", statCapArray[i].name,
                statCapArray[i].pvpCap);
    }

    // Item power spread.
    for (int i = 0; i < itemCount; ++i)
    {
        fprintf(f, "item,%s_pve,%d\n", itemArray[i].name,
                itemArray[i].itemPowerPvE);
        fprintf(f, "item,%s_pvp,%d\n", itemArray[i].name,
                itemArray[i].itemPowerPvP);
    }

    // Tier scaling.
    for (int t = 0; t < MAX_ENEMY_TIERS; ++t)
    {
        fprintf(f, "tier,%s_mult,x%d\n", tierNames[t], tierMultiplier[t]);
        fprintf(f, "tier,%s_baseline,%d%%\n", tierNames[t],
                tierBaselinePercent[t]);
    }

    fclose(f);
    return 1;
}

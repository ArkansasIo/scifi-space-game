# Database and CSV Export

**Module:** `include/db/db.h`, `include/db/schema.h` · `src/db/db.c++`, `src/db/schema.c++`
**Status:** CSV export working; MySQL behind `SBW_ENABLE_MYSQL`

---

## 1. Purpose

The engine keeps every data table in fixed C++ arrays. This module mirrors
those arrays into a relational schema (MySQL) and into spreadsheet-ready CSV
(Excel), and reads them back.

Two audiences:

- **Designers** edit balance in Excel/Google Sheets and import the result.
- **Server operators** persist player state and run telemetry queries.

---

## 2. Configuration

`config/database.ini`:

```ini
[mysql]
enabled = 1
host = 127.0.0.1
port = 3306
user = sbw
password = changeme
database = spacebattlewars
charset = utf8mb4
pool_size = 4
timeout_seconds = 5

[schema]
auto_create = 1
auto_reset = 0

[persistence]
save_framework_on_start = 0
load_framework_on_start = 0
autosave = 1

[excel]
export_dir = data/export
export_on_save = 0
import_items = data/import/items.csv
```

```c
dbDefaultConfig();
dbLoadConfig("config/database.ini");    // overlays the defaults
```

The loader is a hand-written `key = value` parser: it skips comments (`#`,
`;`), trims whitespace on both sides, and ignores unknown keys. A missing file
is not an error — defaults stand.

---

## 3. Schema

24 tables, defined as SQL text in `src/db/schema.c++` (`DB_SCHEMA_SQL`) rather
than inline in code, so the same DDL can be diffed in review, run by
`dbCreateSchema()`, or handed to a DBA.

### 3.1 Content tables

| Table | Rows from | Key columns |
| --- | --- | --- |
| `CoreAttributes` | per character | `str`…`cha` |
| `StatCaps` | `statCapArray[]` | `pve_cap`, `pvp_cap`, `soft_cap`, `curve_k` |
| `Effects` | `effectArray[]` | `category`, `stack_mode`, `dr_category` |
| `EnemyArchetypes` | `archetypeArray[]` | `tier`, `role`, `preferred_range` |
| `Affixes` | `affixArray[]` | `category`, `magnitude`, `stack_cost` |
| `Bosses` | `bossArray[]` | `tier`, `hp_pool`, `soft_enrage_rate` |
| `BossPhases` | `boss.phases[]` | FK → `Bosses`, `hp_threshold` |
| `Items` | `itemArray[]` | `family`, `slot`, `rarity`, `item_power_pve/pvp` |
| `ItemStats` | `pveLines[]` + `pvpLines[]` | FK → `Items`, `context` |
| `GearSets` | `setArray[]` | `bonus2`, `bonus4`, `bonus6` |
| `LootTables` | `lootArray[]` | `pity_timer`, `distribution_rule` |
| `LootEntries` | `loot.entries[]` | FK → `LootTables`, `weight` |
| `Encounters` | `encounterArray[]` | `kind`, `trash_count`, `boss_index` |
| `Factions` | `factionArray[]` | attributes + resources + `at_war` |

### 3.2 World tables

| Table | Rows from |
| --- | --- |
| `StarSystems` | `systemWorldArray[]` |
| `Planets` | FK → `StarSystems` |
| `Ships` | `playerFleet.ships[]` |

### 3.3 Player tables

| Table | Contents |
| --- | --- |
| `Players` | Character state, `ON DUPLICATE KEY UPDATE` for upsert |
| `RunScores` | Per-run score, append-only |
| `Telemetry` | Event stream (`event_id`, `arg_a`, `arg_b`) |

### 3.4 Design notes

**Column names mirror struct fields exactly.** This keeps save/load a
straight field-to-column copy with no mapping layer to maintain.

**Foreign keys cascade.** Deleting a boss removes its phases; deleting a loot
table removes its entries. Child tables can never outlive their parent.

**Drops run in reverse dependency order** (`DB_DROP_SQL`), wrapped in
`SET FOREIGN_KEY_CHECKS = 0/1`, so a reset never fails on constraint order.

**Indexes on every FK column.** `KEY idx_boss (boss_id)` alongside the
constraint, because MySQL does not create one automatically.

---

## 4. Lifecycle

```c
dbConnect();          // 1 on success
dbCreateSchema();     // idempotent: CREATE TABLE IF NOT EXISTS
dbVerifySchema();     // returns tables found
dbDisconnect();
```

```c
dbSaveEverything();   // push all tables
dbLoadEverything();   // pull all tables
```

In the default build (no `SBW_ENABLE_MYSQL`), `dbConnect()` returns 0 and sets:

```
"MySQL support not compiled in (rebuild with -DSBW_ENABLE_MYSQL)"
```

`dbLastError()` returns that string, so the UI can explain the state rather
than silently doing nothing.

---

## 5. Player Persistence

### 5.1 Saving

```c
dbSavePlayer(charactership);
```

Builds an upsert:

```sql
INSERT INTO Players
  (name, power, sif, sp, exp, health, level, lightlevel,
   attackpower, defencepower, weapon, armour, shield)
VALUES (...)
ON DUPLICATE KEY UPDATE power=VALUES(power), sif=VALUES(sif), ...
```

The name is escaped through `dbEscape()` before interpolation. `dbEscape`
handles `'`, `"`, `\`, newline and carriage return, and bounds its output — so
a player named `O'Brien` cannot break the statement.

> **Note:** escaping is used rather than prepared statements because the
> library is not linked in the default build. A live implementation should
> move to `mysql_stmt_*` — `dbEscape` is the single seam to replace.

### 5.2 Run scores

```c
dbSaveRunScore(targetScore);
```

Append-only, with `recorded_at` defaulting to the insert time. Ranking is a
query problem, not a schema problem.

---

## 6. CSV / Excel Export

One function per table:

```c
csvExportDamageTypes(path);   csvExportResources(path);
csvExportStatCaps(path);      csvExportEffects(path);
csvExportArchetypes(path);    csvExportAffixes(path);
csvExportBosses(path);        csvExportItems(path);
csvExportItemPower(path);     csvExportLootTables(path);
csvExportEncounters(path);    csvExportFactions(path);
```

Or everything at once:

```c
csvExportAll("data/export");
```

Writes 12 files into the directory and returns the count written.

### 6.1 Format

Standard CSV with a header row. Text fields containing commas are quoted:

```csv
name,family,slot,rarity,required_level,item_power_pve,item_power_pvp,proc_value_pu
Ashen Blade,0,5,3,40,308,185,90
```

Enum columns export as **integers**, so the header names are the only thing a
designer needs, and the enum definitions live in `core/types.h`.

### 6.2 Why `csvExportItemPower` is separate

`csvExportItems` gives one row per item with its totals. `csvExportItemPower`
explodes the **stat lines**, one row per stat per context:

```
item,context,label,raw,weight,value_pu
"Ashen Blade",PvE,ATK,1200,100,1200
"Ashen Blade",PvE,Fire Damage,120,130,156
"Ashen Blade",PvP,ATK,650,100,650
```

This is the sheet a balance designer actually wants: sort by `value_pu`,
compare PvE against PvP, spot the outliers.

### 6.3 Import

```c
csvImportItems("data/import/items.csv");
```

Reads the file, skips the header, and returns the number of data rows. A real
importer would apply each row over `itemArray[]`; the current version
validates the file format so the pipeline can be tested end to end.

### 6.4 Balance report

```c
csvExportBalanceReport(path);
```

Produces a single long-format file covering caps, item power and tier
scaling:

```
section,key,value
cap,Crit Chance_pve,7000
cap,Crit Chance_pvp,4000
item,Ashen Blade_pve,308
item,Ashen Blade_pvp,185
tier,Elite_mult,x200
tier,Elite_baseline,250%
```

Three sections in one file, because that's the shape a pivot table wants.

---

## 7. API Summary

**Lifecycle**

```c
void dbLoadConfig(const char *path);
void dbDefaultConfig();
const dbconfig &dbGetConfig();
int  dbIsEnabled();
int  dbConnect();
void dbDisconnect();
int  dbIsConnected();
const char *dbLastError();
int  dbExecute(const char *sql);
void dbEscape(const char *in, char out[], int outSize);
```

**Schema**

```c
int dbCreateSchema();
int dbResetSchema();
int dbVerifySchema();
int dbTableCount();
const char *dbTableName(int index);
extern const char *DB_SCHEMA_SQL;
extern const char *DB_DROP_SQL;
```

**Persistence**

```c
int dbSaveStats();       int dbSaveEffects();     int dbSaveArchetypes();
int dbSaveAffixes();     int dbSaveBosses();      int dbSaveItems();
int dbSaveLootTables();  int dbSaveEncounters();  int dbSaveFactions();
int dbSaveEverything();  int dbLoadEverything();
int dbSavePlayer(const player &who);
int dbLoadPlayer(player &who, const char *name);
int dbSaveRunScore(const targetscore &score);
int dbLoadRunScore(targetscore &score);
```

---

## 8. Open Questions

- **Prepared statements.** `dbEscape` is a seam, not a solution. Move to
  `mysql_stmt_*` when the library is actually linked.
- **`dbLoadPlayer` / `dbLoadRunScore` are stubs.** They return 0 rather than
  reading a row, so save/load is write-only today.
- **Migrations.** `dbCreateSchema()` creates missing tables but never alters
  existing ones. A column added to a struct will not appear in an existing
  database.
- **Transaction scope.** `dbSaveEverything()` issues each table separately.
  A failure halfway leaves a partially written database.

// include/db/db.h -- the persistence layer: MySQL and Excel (CSV) export.
//
// The engine keeps every data table in fixed C++ arrays.  This module mirrors
// those arrays into a relational schema (MySQL) and into spreadsheet-ready
// CSV files (Excel), and reads them back.
//
// Like the Lua layer, the MySQL side compiles without a client library:
// set SBW_ENABLE_MYSQL and link libmysqlclient to go live.  The CSV / Excel
// side is fully functional with no external dependency.

#ifndef SBW_DB_DB_H
#define SBW_DB_DB_H

// dbSavePlayer()/dbSaveRunScore() take the original game structs `player` and
// `targetscore`, so this header needs their definitions.  Including core/types.h
// here keeps db.h self-contained: a translation unit can include it directly
// without having to pull in the master header first.
#include "core/types.h"

/* ---------------- configuration ---------------- */

// One row of the connection config.
struct dbconfig
{
    char host[64];
    char port[16];
    char user[64];
    char password[64];
    char database[64];
    char charset[32];
    int enabled;
    int poolSize;
    int timeoutSeconds;
};

// Fill defaults, then overlay config/database.ini if it exists.
void dbLoadConfig(const char *path);
void dbDefaultConfig();

// Access the loaded config.
const dbconfig &dbGetConfig();
int dbIsEnabled();

/* ---------------- MySQL connection ---------------- */

// Connect using the loaded config.  Returns 1 on success.
int dbConnect();
void dbDisconnect();
int dbIsConnected();

// Last error reported by any db* call (never null).
const char *dbLastError();

// Run a statement with no result set (INSERT / UPDATE / CREATE).
int dbExecute(const char *sql);

// Escape a string for safe inclusion in SQL.  Writes into `out`.
void dbEscape(const char *in, char out[], int outSize);

/* ---------------- schema ---------------- */

// Create every table if it does not already exist.
int dbCreateSchema();

// Drop and recreate every table (destructive; used by tests).
int dbResetSchema();

// Validate that every expected table exists.  Returns the count found.
int dbVerifySchema();

/* ---------------- persistence ---------------- */

// Push a framework table into MySQL.  Each returns rows affected, or -1.
int dbSaveStats();
int dbSaveEffects();
int dbSaveArchetypes();
int dbSaveAffixes();
int dbSaveBosses();
int dbSaveItems();
int dbSaveLootTables();
int dbSaveEncounters();
int dbSaveFactions();
int dbSaveEverything();

// Pull a framework table back out of MySQL.  Returns rows read, or -1.
int dbLoadEverything();
int dbLoadItems();
int dbLoadBosses();

/* ---------------- player / run persistence ---------------- */

// Save and load a character and its run score.
int dbSavePlayer(const player &who);
int dbLoadPlayer(player &who, const char *name);
int dbSaveRunScore(const targetscore &score);
int dbLoadRunScore(targetscore &score);

/* ---------------- Excel / CSV ---------------- */

// Export one table to CSV.  Returns rows written, or -1.
int csvExportDamageTypes(const char *path);
int csvExportResources(const char *path);
int csvExportStatCaps(const char *path);
int csvExportEffects(const char *path);
int csvExportArchetypes(const char *path);
int csvExportAffixes(const char *path);
int csvExportBosses(const char *path);
int csvExportItems(const char *path);
int csvExportItemPower(const char *path);
int csvExportLootTables(const char *path);
int csvExportEncounters(const char *path);
int csvExportFactions(const char *path);

// Export every table into `dir` as <name>.csv.  Returns files written.
int csvExportAll(const char *dir);

// Import a CSV back over a table (used for balance passes in Excel).
int csvImportItems(const char *path);

/* ---------------- reporting ---------------- */

// Write a balance report (item power, caps, tier scaling) as CSV.
int csvExportBalanceReport(const char *path);

#endif /* SBW_DB_DB_H */

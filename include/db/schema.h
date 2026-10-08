// include/db/schema.h -- the database schema as SQL text.
//
// Keeping the DDL here (rather than inline in db.c++) means the same strings
// can be dumped to a .sql file for a DBA, run by dbCreateSchema(), or diffed
// in review.

#ifndef SBW_DB_SCHEMA_H
#define SBW_DB_SCHEMA_H

// The complete schema, one CREATE TABLE IF NOT EXISTS per table.
extern const char *DB_SCHEMA_SQL;

// Individual statements, for targeted migrations.
extern const char *DB_DROP_SQL;

// Table names, in creation order.
int dbTableCount();
const char *dbTableName(int index);

// The full list of expected table names, for dbVerifySchema().
extern const char *DB_TABLE_NAMES[];

#endif /* SBW_DB_SCHEMA_H */

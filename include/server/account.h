// include/server/account.h -- accounts, registration and authentication.
//
// Passwords are never stored in plaintext.  Each account stores a random
// salt and a hash derived from (salt + password) with a cheap, dependency-free
// mixing function.  This is deliberately *not* cryptographically strong --
// it exists so the store never contains a readable password, and the design
// leaves one obvious seam (accountHashPassword) to drop in bcrypt/argon2.
//
// See docs/ACCOUNTS.md.

#ifndef SBW_SERVER_ACCOUNT_H
#define SBW_SERVER_ACCOUNT_H

/* ---------------- limits ---------------- */

#define MAX_ACCOUNTS        128
#define ACCOUNT_NAME_MAX    30
#define ACCOUNT_SALT_MAX    24
#define ACCOUNT_HASH_MAX    40

/* ---------------- roles ---------------- */

// Account privilege levels.  Named with an ACCOUNT_ prefix so the enumerators
// do not collide with the combat `enemyrole` enum in core/types.h.
enum accountrole
{
    ACCOUNT_ROLE_PLAYER = 0,
    ACCOUNT_ROLE_MODERATOR,
    ACCOUNT_ROLE_ADMIN,
    ACCOUNT_ROLE_COUNT
};;

/* ---------------- flags ---------------- */

enum accountflags
{
    FLAG_NONE       = 0,
    FLAG_BANNED     = 1 << 0,
    FLAG_MUTED      = 1 << 1,
    FLAG_GM         = 1 << 2
};

/* ---------------- record ---------------- */

struct account
{
    int id;
    int inUse;
    char name[ACCOUNT_NAME_MAX];
    char salt[ACCOUNT_SALT_MAX];
    char hash[ACCOUNT_HASH_MAX];
    accountrole role;
    int flags;
    int level;
    int experience;
    int credits;
    int kills;
    int deaths;
    int highScore;
    int systemIndex;
    int lastLogin;      // tick
    int playTime;       // ticks
    int loginCount;
};

/* ---------------- results ---------------- */

enum accountresult
{
    ACCT_OK = 0,
    ACCT_ERR_NO_SUCH_ACCOUNT,
    ACCT_ERR_BAD_PASSWORD,
    ACCT_ERR_NAME_TAKEN,
    ACCT_ERR_NAME_INVALID,
    ACCT_ERR_PASSWORD_WEAK,
    ACCT_ERR_BANNED,
    ACCT_ERR_STORE_FULL,
    ACCT_ERR_ALREADY_ONLINE,
    ACCT_ERR_IO
};;

const char *accountResultText(int result);

/* ---------------- lifecycle ---------------- */

// Reset the in-memory store.
void accountInit();

// Load accounts from disk.  Returns how many were loaded, or -1 on I/O error.
int accountLoad(const char *path);

// Save accounts to disk.  Returns how many were written, or -1 on I/O error.
int accountSave(const char *path);

// Where the store lives by default.
const char *accountDefaultPath();

/* ---------------- queries ---------------- */

// Find an account by name (case-insensitive).  Returns index, or -1.
int accountFind(const char *name);

// Fetch an account by index.  Returns null when out of range or unused.
account *accountGet(int index);

// Number of registered accounts.
int accountCount();

// Count accounts currently flagged as online.
int accountOnlineCount();

/* ---------------- registration ---------------- */

// Is the name acceptable?  (length, characters, not reserved)
int accountNameIsValid(const char *name);

// Is the password long and mixed enough?
int accountPasswordIsStrong(const char *password);

// Create an account.  Writes the result code into `result` if provided.
// Returns the new account index, or -1.
int accountRegister(const char *name, const char *password, int *result,
                    const char *createdBy);

/* ---------------- authentication ---------------- */

// Verify credentials.  On success marks the account online and returns its
// index.  Writes the result code into `result` if provided.
int accountLogin(const char *name, const char *password, int *result);

// Mark an account offline.
int accountLogout(const char *name);

// Is this account online right now?
int accountIsOnline(const char *name);

/* ---------------- maintenance ---------------- */

// Change a password after verifying the old one.
int accountChangePassword(const char *name, const char *oldPassword,
                          const char *newPassword);

// Set a flag on or off (ban, mute, gm).
int accountSetFlag(const char *name, int flag, int enabled);

// Test a flag.
int accountHasFlag(const char *name, int flag);

// Delete an account.
int accountDelete(const char *name);

/* ---------------- progression persistence ---------------- */

// Persist a player's run result into their account.
void accountRecordResult(const char *name, int level, int experience,
                         int score, int kills, int deaths);

// Rank a score against every account's personal best.  1 = best.
int accountScoreRank(int score);

/* ---------------- the top-ten board ---------------- */

// Fill `out` with the indices of the highest-scoring accounts.
// Returns how many were written.
int accountTopScores(int out[], int outMax);

/* ---------------- hashing ---------------- */

// Mix (salt + password) into a hex digest written into `out`.
// The one seam to replace with a real KDF.
void accountHashPassword(const char *salt, const char *password,
                         char out[], int outSize);

// Generate a fresh random salt into `out`.
void accountGenerateSalt(char out[], int outSize);

/* ---------------- clock ---------------- */

// The store measures play time in ticks.  The server drives this.
void accountTick(int ticks);
int accountCurrentTick();

#endif /* SBW_SERVER_ACCOUNT_H */

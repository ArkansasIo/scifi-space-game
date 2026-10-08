// src/server/account.c++ -- accounts, registration and authentication.
//
// An in-memory store with a flat-file persistence format:
//
//   id|name|salt|hash|role|flags|level|exp|credits|kills|deaths|high|system
//
// Passwords are salted and mixed, never stored readable.  See docs/ACCOUNTS.md.

#include "spacebattlerpg.h"
#include "server/account.h"
#include "server/protocol.h"   // for PROTO_MAX_LINE

/* ---------------- storage ---------------- */

static account g_accounts[MAX_ACCOUNTS];
static int g_nextAccountId = 1;
static int g_tick = 0;

/* ---------------- helpers ---------------- */

static void copyString(char *dest, int destSize, const char *src)
{
    if (!dest || destSize <= 0)
        return;

    strncpy(dest, src ? src : "", destSize - 1);
    dest[destSize - 1] = '\0';
}

// Case-insensitive comparison, used for every name lookup so "Khan" and
// "khan" are the same account.  Implemented locally rather than with
// _stricmp/strcasecmp, which are not portable across mingw and MSVC.
static int nameEquals(const char *a, const char *b)
{
    if (!a || !b)
        return 0;

    while (*a && *b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;

        a++;
        b++;
    }

    return (*a == '\0' && *b == '\0') ? 1 : 0;
}

static void nameLower(char *out, int outSize, const char *in)
{
    if (!out || outSize <= 0)
        return;

    int i = 0;

    for (; in && in[i] && i < outSize - 1; ++i)
        out[i] = (char)tolower((unsigned char)in[i]);

    out[i] = '\0';
}

/* ---------------- result text ---------------- */

const char *accountResultText(int result)
{
    switch (result)
    {
    case ACCT_OK:
        return "ok";
    case ACCT_ERR_NO_SUCH_ACCOUNT:
        return "no such account";
    case ACCT_ERR_BAD_PASSWORD:
        return "incorrect password";
    case ACCT_ERR_NAME_TAKEN:
        return "that name is already taken";
    case ACCT_ERR_NAME_INVALID:
        return "that name is not allowed";
    case ACCT_ERR_PASSWORD_WEAK:
        return "that password is too weak (8+ chars, letters and digits)";
    case ACCT_ERR_BANNED:
        return "this account is banned";
    case ACCT_ERR_STORE_FULL:
        return "the account store is full";
    case ACCT_ERR_ALREADY_ONLINE:
        return "that account is already logged in";
    case ACCT_ERR_IO:
        return "storage error";
    default:
        return "unknown error";
    }
}

/* ---------------- hashing ---------------- */

void accountGenerateSalt(char out[], int outSize)
{
    if (!out || outSize <= 0)
        return;

    // Two halfwords of entropy, rendered as hex.
    unsigned int a = (unsigned int)(time(0) ^ (rand() << 16));
    unsigned int b = (unsigned int)(rand() ^ (rand() << 8) ^ clock());

    snprintf(out, outSize, "%08x%08x", a, b);
}

void accountHashPassword(const char *salt, const char *password,
                         char out[], int outSize)
{
    if (!out || outSize <= 0)
        return;

    // FNV-1a over (salt + password), then a few mixing rounds.  This is a
    // deliberate placeholder for a real KDF: it is one seam, called from
    // exactly two places (register and login), so swapping in bcrypt/argon2
    // is a single-function change.
    unsigned int hash = 2166136261u;

    const char *parts[2];
    parts[0] = salt ? salt : "";
    parts[1] = password ? password : "";

    for (int p = 0; p < 2; ++p)
    {
        for (const char *c = parts[p]; *c; ++c)
        {
            hash ^= (unsigned char)*c;
            hash *= 16777619u;
        }

        // Separator between salt and password.
        hash ^= 0x1Fu;
        hash *= 16777619u;
    }

    // Avalanche rounds so a one-character change flips many bits.
    for (int round = 0; round < 8; ++round)
    {
        hash ^= hash >> 13;
        hash *= 0x5bd1e995u;
        hash ^= hash >> 15;
    }

    snprintf(out, outSize, "%08x%08x", hash, hash ^ 0xA5A5A5A5u);
}

/* ---------------- lifecycle ---------------- */

void accountInit()
{
    memset(g_accounts, 0, sizeof(g_accounts));

    for (int i = 0; i < MAX_ACCOUNTS; ++i)
        g_accounts[i].id = -1;

    g_nextAccountId = 1;
    g_tick = 0;
}

const char *accountDefaultPath()
{
    return "data/accounts.dat";
}

int accountCount()
{
    int count = 0;

    for (int i = 0; i < MAX_ACCOUNTS; ++i)
    {
        if (g_accounts[i].inUse)
            count++;
    }

    return count;
}

int accountOnlineCount()
{
    int count = 0;

    for (int i = 0; i < MAX_ACCOUNTS; ++i)
    {
        if (g_accounts[i].inUse && g_accounts[i].lastLogin >= 0
            && accountIsOnline(g_accounts[i].name))
            count++;
    }

    return count;
}

/* ---------------- load / save ---------------- */

int accountLoad(const char *path)
{
    if (!path)
        path = accountDefaultPath();

    FILE *f = fopen(path, "rb");

    if (!f)
        return 0;   // no store yet is not an error

    char line[PROTO_MAX_LINE];
    int loaded = 0;

    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;

        int id = 0;
        int role = 0;
        int flags = 0;
        int level = 0;
        int exp = 0;
        int credits = 0;
        int kills = 0;
        int deaths = 0;
        int high = 0;
        int system = 0;

        char name[ACCOUNT_NAME_MAX] = {0};
        char salt[ACCOUNT_SALT_MAX] = {0};
        char hash[ACCOUNT_HASH_MAX] = {0};

        int fields = sscanf(line, "%d|%29[^|]|%23[^|]|%39[^|]|%d|%d|%d|%d|%d|%d|%d|%d|%d",
                            &id, name, salt, hash, &role, &flags, &level,
                            &exp, &credits, &kills, &deaths, &high, &system);

        if (fields < 13)
            continue;

        // Find a free slot.
        for (int i = 0; i < MAX_ACCOUNTS; ++i)
        {
            if (g_accounts[i].inUse)
                continue;

            account &acct = g_accounts[i];
            memset(&acct, 0, sizeof(acct));

            acct.inUse = 1;
            acct.id = id;
            copyString(acct.name, sizeof(acct.name), name);
            copyString(acct.salt, sizeof(acct.salt), salt);
            copyString(acct.hash, sizeof(acct.hash), hash);
            acct.role = (accountrole)role;
            acct.flags = flags;
            acct.level = level;
            acct.experience = exp;
            acct.credits = credits;
            acct.kills = kills;
            acct.deaths = deaths;
            acct.highScore = high;
            acct.systemIndex = system;
            acct.lastLogin = -1;

            if (id >= g_nextAccountId)
                g_nextAccountId = id + 1;

            loaded++;
            break;
        }
    }

    fclose(f);
    return loaded;
}

int accountSave(const char *path)
{
    if (!path)
        path = accountDefaultPath();

    FILE *f = fopen(path, "wb");

    if (!f)
        return -1;

    fprintf(f, "# spacebattlerpg account store\n");
    fprintf(f, "# id|name|salt|hash|role|flags|level|exp|credits|"
               "kills|deaths|high|system\n");

    int written = 0;

    for (int i = 0; i < MAX_ACCOUNTS; ++i)
    {
        const account &acct = g_accounts[i];

        if (!acct.inUse)
            continue;

        fprintf(f, "%d|%s|%s|%s|%d|%d|%d|%d|%d|%d|%d|%d|%d\n",
                acct.id, acct.name, acct.salt, acct.hash,
                (int)acct.role, acct.flags, acct.level, acct.experience,
                acct.credits, acct.kills, acct.deaths, acct.highScore,
                acct.systemIndex);

        written++;
    }

    fclose(f);
    return written;
}

/* ---------------- queries ---------------- */

int accountFind(const char *name)
{
    if (!name || name[0] == '\0')
        return -1;

    for (int i = 0; i < MAX_ACCOUNTS; ++i)
    {
        if (g_accounts[i].inUse && nameEquals(g_accounts[i].name, name))
            return i;
    }

    return -1;
}

account *accountGet(int index)
{
    if (index < 0 || index >= MAX_ACCOUNTS)
        return 0;

    if (!g_accounts[index].inUse)
        return 0;

    return &g_accounts[index];
}

/* ---------------- validation ---------------- */

int accountNameIsValid(const char *name)
{
    if (!name)
        return 0;

    int length = (int)strlen(name);

    if (length < 3 || length >= ACCOUNT_NAME_MAX)
        return 0;

    for (int i = 0; i < length; ++i)
    {
        unsigned char c = (unsigned char)name[i];

        // Letters, digits, underscore and hyphen only.
        if (!isalnum(c) && c != '_' && c != '-')
            return 0;
    }

    // Reserved names that would confuse commands or impersonate staff.
    static const char *reserved[] = {
        "admin", "administrator", "gm", "mod", "moderator",
        "system", "server", "root", "null", "none"};

    char lower[ACCOUNT_NAME_MAX];
    nameLower(lower, sizeof(lower), name);

    for (int i = 0; i < (int)(sizeof(reserved) / sizeof(reserved[0])); ++i)
    {
        if (strcmp(lower, reserved[i]) == 0)
            return 0;
    }

    return 1;
}

int accountPasswordIsStrong(const char *password)
{
    if (!password)
        return 0;

    int length = (int)strlen(password);

    if (length < 8)
        return 0;

    int hasLetter = 0;
    int hasDigit = 0;

    for (int i = 0; i < length; ++i)
    {
        unsigned char c = (unsigned char)password[i];

        if (isalpha(c))
            hasLetter = 1;
        else if (isdigit(c))
            hasDigit = 1;
    }

    return (hasLetter && hasDigit) ? 1 : 0;
}

/* ---------------- registration ---------------- */

int accountRegister(const char *name, const char *password, int *result,
                    const char *createdBy)
{
    if (result)
        *result = ACCT_OK;

    if (!accountNameIsValid(name))
    {
        if (result)
            *result = ACCT_ERR_NAME_INVALID;
        return -1;
    }

    if (accountFind(name) >= 0)
    {
        if (result)
            *result = ACCT_ERR_NAME_TAKEN;
        return -1;
    }

    if (!accountPasswordIsStrong(password))
    {
        if (result)
            *result = ACCT_ERR_PASSWORD_WEAK;
        return -1;
    }

    // Find a free slot.
    int slot = -1;

    for (int i = 0; i < MAX_ACCOUNTS; ++i)
    {
        if (!g_accounts[i].inUse)
        {
            slot = i;
            break;
        }
    }

    if (slot < 0)
    {
        if (result)
            *result = ACCT_ERR_STORE_FULL;
        return -1;
    }

    account &acct = g_accounts[slot];
    memset(&acct, 0, sizeof(acct));

    acct.inUse = 1;
    acct.id = g_nextAccountId++;

    copyString(acct.name, sizeof(acct.name), name);

    accountGenerateSalt(acct.salt, sizeof(acct.salt));
    accountHashPassword(acct.salt, password, acct.hash, sizeof(acct.hash));

    // The first account created becomes the admin.
    if (accountCount() == 1 && !createdBy)
    {
        acct.role = ACCOUNT_ROLE_ADMIN;
        acct.flags = FLAG_GM;
    }
    else
    {
        acct.role = ACCOUNT_ROLE_PLAYER;
    }

    acct.level = 1;
    acct.experience = 0;
    acct.credits = 100;
    acct.kills = 0;
    acct.deaths = 0;
    acct.highScore = 0;
    acct.systemIndex = 0;
    acct.lastLogin = -1;
    acct.playTime = 0;
    acct.loginCount = 0;

    return slot;
}

/* ---------------- authentication ---------------- */

int accountLogin(const char *name, const char *password, int *result)
{
    if (result)
        *result = ACCT_OK;

    int index = accountFind(name);

    if (index < 0)
    {
        if (result)
            *result = ACCT_ERR_NO_SUCH_ACCOUNT;
        return -1;
    }

    account &acct = g_accounts[index];

    if (acct.flags & FLAG_BANNED)
    {
        if (result)
            *result = ACCT_ERR_BANNED;
        return -1;
    }

    char candidate[ACCOUNT_HASH_MAX];
    accountHashPassword(acct.salt, password, candidate, sizeof(candidate));

    if (strcmp(candidate, acct.hash) != 0)
    {
        if (result)
            *result = ACCT_ERR_BAD_PASSWORD;
        return -1;
    }

    acct.lastLogin = g_tick;
    acct.loginCount++;

    return index;
}

int accountLogout(const char *name)
{
    int index = accountFind(name);

    if (index < 0)
        return 0;

    account &acct = g_accounts[index];

    if (acct.lastLogin >= 0)
    {
        acct.playTime += g_tick - acct.lastLogin;
        acct.lastLogin = -1;
    }

    return 1;
}

int accountIsOnline(const char *name)
{
    int index = accountFind(name);

    if (index < 0)
        return 0;

    return (g_accounts[index].lastLogin >= 0) ? 1 : 0;
}

/* ---------------- maintenance ---------------- */

int accountChangePassword(const char *name, const char *oldPassword,
                          const char *newPassword)
{
    int index = accountFind(name);

    if (index < 0)
        return ACCT_ERR_NO_SUCH_ACCOUNT;

    account &acct = g_accounts[index];

    char candidate[ACCOUNT_HASH_MAX];
    accountHashPassword(acct.salt, oldPassword, candidate, sizeof(candidate));

    if (strcmp(candidate, acct.hash) != 0)
        return ACCT_ERR_BAD_PASSWORD;

    if (!accountPasswordIsStrong(newPassword))
        return ACCT_ERR_PASSWORD_WEAK;

    // Re-salt on every change, so an old hash can never be replayed.
    accountGenerateSalt(acct.salt, sizeof(acct.salt));
    accountHashPassword(acct.salt, newPassword, acct.hash, sizeof(acct.hash));

    return ACCT_OK;
}

int accountSetFlag(const char *name, int flag, int enabled)
{
    int index = accountFind(name);

    if (index < 0)
        return 0;

    if (enabled)
        g_accounts[index].flags |= flag;
    else
        g_accounts[index].flags &= ~flag;

    return 1;
}

int accountHasFlag(const char *name, int flag)
{
    int index = accountFind(name);

    if (index < 0)
        return 0;

    return (g_accounts[index].flags & flag) ? 1 : 0;
}

int accountDelete(const char *name)
{
    int index = accountFind(name);

    if (index < 0)
        return 0;

    g_accounts[index].inUse = 0;
    g_accounts[index].lastLogin = -1;

    return 1;
}

/* ---------------- progression persistence ---------------- */

void accountRecordResult(const char *name, int level, int experience,
                         int score, int kills, int deaths)
{
    int index = accountFind(name);

    if (index < 0)
        return;

    account &acct = g_accounts[index];

    acct.level = level;
    acct.experience = experience;
    acct.kills += kills;
    acct.deaths += deaths;

    if (score > acct.highScore)
        acct.highScore = score;

    // Credits are earned from the run at a bounded rate.
    acct.credits += score / 10;
}

int accountScoreRank(int score)
{
    // 1 = best.  A player beats every account whose best is lower.
    int better = 0;

    for (int i = 0; i < MAX_ACCOUNTS; ++i)
    {
        if (g_accounts[i].inUse && g_accounts[i].highScore > score)
            better++;
    }

    return better + 1;
}

int accountTopScores(int out[], int outMax)
{
    if (!out || outMax <= 0)
        return 0;

    // Selection sort over the (small) account table, highest first.
    int used[MAX_ACCOUNTS];
    int usedCount = 0;

    for (int i = 0; i < MAX_ACCOUNTS; ++i)
    {
        if (g_accounts[i].inUse)
            used[usedCount++] = i;
    }

    int written = 0;

    while (written < outMax)
    {
        int best = -1;
        int bestScore = -1;

        for (int i = 0; i < usedCount; ++i)
        {
            if (used[i] < 0)
                continue;

            if (g_accounts[used[i]].highScore > bestScore)
            {
                bestScore = g_accounts[used[i]].highScore;
                best = i;
            }
        }

        if (best < 0)
            break;

        if (bestScore <= 0 && written > 0)
            break;

        out[written] = used[best];
        written++;
        used[best] = -1;
    }

    return written;
}

/* ---------------- clock ---------------- */

// The account store needs a tick to measure play time.  The server drives it.
void accountTick(int ticks)
{
    g_tick += ticks;
}

int accountCurrentTick()
{
    return g_tick;
}

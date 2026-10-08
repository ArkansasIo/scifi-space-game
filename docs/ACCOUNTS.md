# Accounts

**Module:** `include/server/account.h` · `src/server/account.c++`
**Status:** implemented

---

## 1. Purpose

Registration, authentication and persistent progression. An account is what
survives between sessions: level, credits, kills, personal best, and which
system the player logged out in.

---

## 2. Security Model

> **Read this before shipping.** The password hashing here is a deliberate
> placeholder, not a real KDF.

Passwords are never stored readable. Each account stores:

- a random `salt` (16 hex chars from `time()`, `rand()` and `clock()`)
- a `hash` derived from `salt + password`

The hash is FNV-1a followed by eight avalanche rounds — fast, dependency-free,
and **not** resistant to a determined offline attack. It exists so:

1. the flat-file store never contains a plaintext password
2. there is exactly **one** function to replace

`accountHashPassword(salt, password, out, size)` is called from exactly two
places (register and login) and `accountChangePassword()`. Swapping in
bcrypt/argon2 is a single-function change with no call-site edits.

---

## 3. Account Record

```c
struct account
{
    int id;
    int inUse;
    char name[ACCOUNT_NAME_MAX];      // 30
    char salt[ACCOUNT_SALT_MAX];      // 24
    char hash[ACCOUNT_HASH_MAX];      // 40
    accountrole role;
    int flags;
    int level, experience, credits;
    int kills, deaths, highScore;
    int systemIndex;
    int lastLogin;      // tick; -1 when offline
    int playTime;       // ticks
    int loginCount;
};
```

Capacity: 128 accounts (`MAX_ACCOUNTS`).

### 3.1 Roles

| Role | Meaning |
| --- | --- |
| `ACCOUNT_ROLE_PLAYER` | Default |
| `ACCOUNT_ROLE_MODERATOR` | Can mute, kick |
| `ACCOUNT_ROLE_ADMIN` | Full access |

### 3.2 Flags

| Flag | Effect |
| --- | --- |
| `FLAG_BANNED` | Login refused with `ACCT_ERR_BANNED` |
| `FLAG_MUTED` | Cannot send chat (enforced by the caller) |
| `FLAG_GM` | Shown with a GM tag; may use admin verbs |

**The first account created becomes the admin.** This is intentional — a fresh
install needs a way in — but it means the very first registration on a public
server claims it. Seed an admin before opening the server.

---

## 4. Validation

### 4.1 Names

`accountNameIsValid()` requires:

- length 3 to 29
- characters: letters, digits, `_`, `-` only
- not a reserved word

Reserved: `admin`, `administrator`, `gm`, `mod`, `moderator`, `system`,
`server`, `root`, `null`, `none`.

Lookups are **case-insensitive** (`Khan` and `khan` are the same account), so
the reserved check lowercases before comparing.

### 4.2 Passwords

`accountPasswordIsStrong()` requires:

- at least 8 characters
- at least one letter
- at least one digit

Deliberately modest. The check exists to stop `abc123`, not to enforce a
policy that drives players to post-it notes.

---

## 5. Registration

```c
int *result;
int index = accountRegister("Khan", "warpcore9", &result, NULL);
```

Returns the account index, or `-1` with `result` set:

| Result | Cause |
| --- | --- |
| `ACCT_ERR_NAME_INVALID` | Failed name rules |
| `ACCT_ERR_NAME_TAKEN` | Name already registered |
| `ACCT_ERR_PASSWORD_WEAK` | Failed password rules |
| `ACCT_ERR_STORE_FULL` | 128 accounts already |

---

## 6. Authentication

```c
int *result;
int index = accountLogin("Khan", "warpcore9", &result);
```

| Result | Cause |
| --- | --- |
| `ACCT_OK` | Success; `lastLogin` set, `loginCount` incremented |
| `ACCT_ERR_NO_SUCH_ACCOUNT` | Unknown name |
| `ACCT_ERR_BANNED` | Flagged |
| `ACCT_ERR_BAD_PASSWORD` | Hash mismatch |

Note the ban check happens **before** the hash comparison, so a banned account
is refused without doing hashing work.

### 6.1 Online state

"Online" is modelled as `lastLogin >= 0`, not a separate boolean — one field
cannot disagree with itself.

```c
accountLogin()  → lastLogin = currentTick
accountLogout() → playTime += (currentTick - lastLogin); lastLogin = -1
```

`accountIsOnline(name)` derives the state from that field.

### 6.2 Password change

`accountChangePassword()` verifies the old password, validates the new one,
and then **re-salts**. A fresh salt on every change means an attacker who
captured an old hash cannot replay it after a change.

---

## 7. Progression Persistence

```c
void accountRecordResult(const char *name, int level, int experience,
                         int score, int kills, int deaths);
```

Called at the end of a run. It:

- overwrites `level` and `experience` (they're current, not cumulative)
- **accumulates** `kills` and `deaths` (lifetime totals)
- raises `highScore` if beaten
- grants `credits += score / 10` (bounded, so score cannot inflate the
  economy)

### 7.1 Scoring

```c
int accountScoreRank(int score);      // 1 = best
int accountTopScores(int out[], int outMax);
```

`accountScoreRank()` counts accounts with a higher best and adds one — so a
player's rank is meaningful even before the leaderboard is displayed.
`accountTopScores()` is a selection sort over the (small) table, highest
first, skipping zero-score accounts after the first entry.

---

## 8. Persistence

Flat file, pipe-delimited, one account per line:

```
# spacebattlerpg account store
# id|name|salt|hash|role|flags|level|exp|credits|kills|deaths|high|system
1|Khan|a1b2c3d4e5f60718|9f8e7d6c5b4a3928|2|4|42|1800|4500|210|7|9600|11
```

```c
accountInit();
accountLoad("data/accounts.dat");    // returns count loaded, 0 if absent
accountSave("data/accounts.dat");    // returns count written, -1 on I/O error
```

A missing file is **not an error** — a fresh install has no store yet, and
`accountLoad` returns 0.

Fields are parsed with `sscanf` using `%[^|]` conversions, and any line with
fewer than 13 fields is skipped rather than partially applied.

---

## 9. API Summary

```c
void accountInit();
int  accountLoad(const char *path);
int  accountSave(const char *path);
const char *accountDefaultPath();

int  accountFind(const char *name);
account *accountGet(int index);
int  accountCount();
int  accountOnlineCount();

int  accountNameIsValid(const char *name);
int  accountPasswordIsStrong(const char *password);
int  accountRegister(const char *name, const char *password, int *result,
                     const char *createdBy);

int  accountLogin(const char *name, const char *password, int *result);
int  accountLogout(const char *name);
int  accountIsOnline(const char *name);

int  accountChangePassword(const char *name, const char *oldPassword,
                           const char *newPassword);
int  accountSetFlag(const char *name, int flag, int enabled);
int  accountHasFlag(const char *name, int flag);
int  accountDelete(const char *name);

void accountRecordResult(const char *name, int level, int experience,
                         int score, int kills, int deaths);
int  accountScoreRank(int score);
int  accountTopScores(int out[], int outMax);

void accountHashPassword(const char *salt, const char *password,
                         char out[], int outSize);
void accountGenerateSalt(char out[], int outSize);

void accountTick(int ticks);
int  accountCurrentTick();
```

---

## 10. Open Questions

- **The KDF.** Replace before any public deployment. This is called out in
  the header too, so nobody has to find this document to discover it.
- **Duplicate concurrent logins.** `ACCT_ERR_ALREADY_ONLINE` is declared but
  `accountLogin()` does not refuse a second login. The check belongs there,
  gated on `allowDuplicateNames`.
- **Data races.** The store is global mutable state with no locking. Fine for
  a single-threaded server; a threaded transport needs a mutex around every
  accessor.
- **Store size.** 128 accounts is a placeholder. The flat-file format would
  become the bottleneck well before a real database would.

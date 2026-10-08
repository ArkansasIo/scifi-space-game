# Server

**Module:** `include/server/server.h` · `src/server/server.c++`
**Status:** headers implemented; `server.c++` not yet written

---

## 1. Purpose

The authoritative game host. It owns sessions, the world state clients are
allowed to see, and the command dispatch that turns a protocol line into a
response.

Transport is deliberately abstracted: the default build runs an **in-process
loop** (useful for tests and single-player hosting), and a socket transport
can be dropped in behind the same interface. Nothing in the server core knows
what a socket is.

---

## 2. Configuration

```c
struct serverconfig
{
    char host[64];
    int port;                 // default 27666
    int maxSessions;          // default 64
    int tickRate;             // updates per second
    char motd[256];
    int allowDuplicateNames;
    int pvpEnabled;
    int logLevel;             // 0 quiet, 1 normal, 2 verbose
};
```

Loaded from an ini file, falling back to defaults:

```c
serverDefaultConfig();
serverLoadConfig("config/server.ini");
```

---

## 3. Sessions

```c
enum sessionstate
{
    SESSION_FREE = 0,
    SESSION_CONNECTED,     // transport up, not authenticated
    SESSION_AUTHED,        // logged in, in the world
    SESSION_IN_COMBAT,     // locked to an encounter
    SESSION_CLOSING        // flushing, then freed
};

struct session
{
    int id;
    sessionstate state;
    char name[30];
    int systemIndex;       // where the player currently is
    int level, experience;
    int hp, maxHp, power;
    int kills, score;
    int inCombat;
    int encounterIndex;
    int lastRoll;
    unsigned int seed;     // per-session RNG stream
};
```

### 3.1 Why per-session seeds

Each session carries its own `seed`. Loot rolls, encounter generation and
random events draw from the **session's** stream, not a global one. Two
consequences:

- One player's rolls cannot be predicted by watching another's.
- A session is reproducible from its seed, which makes bug reports actionable.

### 3.2 Session lifetime

```
serverConnect()   → allocates a slot, state = SESSION_CONNECTED
   ↓ LOGIN
accountLogin()    → state = SESSION_AUTHED
   ↓ ENCOUNTER
                  → state = SESSION_IN_COMBAT
   ↓ LOGOUT / QUIT
serverDisconnect()→ slot freed, account marked offline, state = SESSION_FREE
```

`maxSessions` caps concurrent slots; a connect beyond the cap returns
`ERR_SERVER_FULL` rather than queuing.

---

## 4. Dispatch

```c
int serverHandleLine(int sessionId, const char *line,
                     char out[], int outSize);
int serverHandleMessage(int sessionId, const protomessage &msg,
                        char out[], int outSize);
```

`serverHandleLine()` parses then delegates, so the parsed-message variant can
be called directly by tests without going through string formatting.

### 4.1 Authorization

`protoRequiresLogin(verb)` gates verbs by session state. Calling `TRAVEL`
before `LOGIN` returns `ERR_NOT_LOGGED_IN` rather than reaching the world
logic:

| Verb | Requires |
| --- | --- |
| `HELLO`, `LOGIN`, `PING`, `QUIT`, `MOTD`, `HELP` | nothing |
| everything else | `SESSION_AUTHED` or later |

### 4.2 Error codes

| Code | Meaning |
| --- | --- |
| 1 | Bad syntax |
| 2 | Unknown verb |
| 3 | Not logged in |
| 4 | Already logged in |
| 5 | No such system |
| 6 | No such sector |
| 7 | Unreachable (no gate route) |
| 8 | No such loot table |
| 9 | Not in combat |
| 10 | Server full |
| 11 | Internal error |

---

## 5. Update Loop

```c
serverStart();
while (running)
{
    serverUpdate(1);                       // world tick
    schedulerTick(1, turnNumber());        // tick-scheduled jobs
    // poll transport, dispatch each line
}
serverStop();
```

`serverUpdate()` advances world state that isn't tied to a turn: session
timeouts, combat timers, tick-scheduled jobs.

---

## 6. Statistics

```c
struct serverstats
{
    int sessionsPeak;
    int commandsHandled;
    int errorsReturned;
    int loginsTotal;
    int fightsResolved;
    int uptimeTicks;
};
```

Exposed via `serverGetStats()` and shown by a `WHO`-adjacent admin path.
`errorsReturned` climbing while `commandsHandled` is flat is the signal that a
client is speaking a stale protocol.

---

## 7. Logging

```c
serverLog(int level, const char *message);
```

Levels match `serverconfig.logLevel`: 0 quiet, 1 normal, 2 verbose. Combat
resolution and loot rolls log at level 2 because they're the noisy ones.

---

## 8. API Summary

```c
void serverLoadConfig(const char *path);
void serverDefaultConfig();
const serverconfig &serverGetConfig();
int  serverStart();
void serverStop();
void serverUpdate(int ticks);
int  serverIsRunning();

int  serverConnect();
void serverDisconnect(int sessionId);
session *serverGetSession(int sessionId);
int  serverFindSessionByName(const char *name);
int  serverSessionCount();
const session *serverSessions();
int  serverSessionCapacity();

int  serverHandleLine(int sessionId, const char *line,
                      char out[], int outSize);
int  serverHandleMessage(int sessionId, const protomessage &msg,
                         char out[], int outSize);

const serverstats &serverGetStats();
void serverResetStats();
void serverLog(int level, const char *message);
```

---

## 9. Open Questions

- **Socket transport.** The interface is ready but no listener exists. The
  first real transport should be its own module behind `serverStart()`.
- **Persistence cadence.** Sessions hold live state but nothing currently
  flushes it to the account store on disconnect. `serverDisconnect()` is the
  obvious hook.
- **Anti-cheat.** All validation is server-side by construction, but there is
  no rate limiting — a client can spam verbs as fast as the transport accepts
  them.

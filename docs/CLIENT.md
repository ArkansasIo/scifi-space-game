# Client

**Module:** `include/client/client.h` · `src/client/client.c++`
**Status:** header implemented; `client.c++` and `cli.c++` not yet written

---

## 1. Purpose

A thin, transport-agnostic client. It formats a request, takes a response, and
exposes the world state it has learned. The CLI front end drives this; a GUI
would drive the same API without changes to the core.

The client is **not** a second implementation of the game rules. It sends
verbs and prints responses — all authority stays on the server.

---

## 2. Configuration

```c
struct clientconfig
{
    char host[64];
    int port;
    int timeoutSeconds;
    int colorEnabled;
    int autoReconnect;
};
```

```c
clientDefaultConfig();
clientLoadConfig("config/client.ini");
```

---

## 3. Connection

```c
int clientConnect(const char *host, int port);
void clientDisconnect();
int clientIsConnected();
int clientIsLoggedIn();
```

In the in-process build, `clientConnect` binds the client to the local server
core. A socket transport would dial instead — the call site does not change.

---

## 4. Cached State

```c
struct clientstate
{
    int connected;
    int loggedIn;
    char playerName[30];
    int level, hp, maxHp, power;
    int score, kills;
    int currentSystem;
    char currentSystemName[30];
    int inCombat;
};
```

The cache is what the **renderer** reads. The CLI never re-queries the server
to draw a status bar — it reads `clientGetState()`. State updates when a
response arrives that informs it.

This is the standard client/server split: responses are authoritative, the
cache is a local mirror, and a stale cache is a display problem rather than a
correctness problem.

---

## 5. Request / Response

```c
int clientSend(const char *line, char response[], int responseSize);
```

Returns 1 on transport success, 0 on failure. Note it reports **transport**
success, not command success — `ERR 7 unreachable` is a successful round trip.

### 5.1 Wrappers

One per verb, so the CLI never formats a raw line:

```c
clientHello();            clientLogin(name);       clientPing();
clientWhoAmI();           clientStats();           clientUniverse();
clientSectors();          clientSystems();         clientSector(i);
clientSystem(i);          clientTravel(i);         clientEncounter(tier);
clientFight();            clientFlee();            clientLoot(table);
clientStory();            clientWho();             clientRoll(sides);
clientSeed(n);            clientMotd();            clientHelp();
```

---

## 6. Command Line

### 6.1 Tokenizing

```c
char tokens[12][128];
int count = clientTokenize("travel 12", tokens, 12);
// count = 2, tokens[0] = "travel", tokens[1] = "12"
```

### 6.2 Dispatch

```c
int clientRunCommand(const char *line);
```

Returns 0 to keep the session open, 1 to quit. Handles the **local** commands
that never reach the server (`help`, `quit`, `clear`) and forwards everything
else.

This local/remote split matters: a client that sent `help` to the server would
fail whenever the server was down, which is precisely when a player needs help
most.

### 6.3 The shell

```c
int clientRunShell();
```

Reads a line, runs it, prints the result, repeats until quit. Uses the same
`readChar`/`getline` discipline as the rest of the project, so a pasted line
cannot desync the next prompt.

---

## 7. CLI Command Set

| Command | Sends | Notes |
| --- | --- | --- |
| `help` | — | Local |
| `quit` | `LOGOUT` | Then exits |
| `clear` | — | Local |
| `connect` | — | Local (binds transport) |
| `login <name>` | `LOGIN` | |
| `whoami` | `WHOAMI` | |
| `stats` | `STATS` | |
| `universe` | `UNIVERSE` | |
| `sectors` | — | Via `SECTOR` / list |
| `systems` | `SYSTEMS` | |
| `sector <n>` | `SECTOR n` | |
| `system <n>` | `SYSTEM n` | |
| `travel <n>` | `TRAVEL n` | Server validates reachability |
| `encounter [tier]` | `ENCOUNTER` | |
| `fight` | `FIGHT` | |
| `flee` | `FLEE` | |
| `loot <table>` | `LOOT` | |
| `story` | `STORY` | |
| `who` | `WHO` | |
| `roll <sides>` | `ROLL` | Server rolls, not client |
| `seed <n>` | `SEED` | |
| `motd` | `MOTD` | |

**`roll` going to the server** is the deliberate design point: a client-side
roll is trivially cheatable, and the server already has a per-session RNG
stream (`session.seed`).

---

## 8. Typical Session

```
$ spacebattlerpg --client
> connect
connected to 127.0.0.1:27666

> login Khan
welcome Khan

> whoami
Khan level=1 hp=100/100 power=100 system=0

> travel 1
Altair IV

> encounter
4 units, budget 2400

> fight
victory, 3 kills, +300 score

> quit
goodbye
```

---

## 9. API Summary

```c
void clientLoadConfig(const char *path);
void clientDefaultConfig();
const clientconfig &clientGetConfig();

int  clientConnect(const char *host, int port);
void clientDisconnect();
int  clientIsConnected();
int  clientIsLoggedIn();
const clientstate &clientGetState();

int  clientSend(const char *line, char response[], int responseSize);
/* + one wrapper per verb */

int  clientTokenize(const char *line, char tokens[][128], int maxTokens);
int  clientRunCommand(const char *line);
int  clientRunShell();
void clientPrintHelp();
```

---

## 10. Open Questions

- **Reconnect.** `autoReconnect` is declared but unused. Reconnect needs a
  resumed session or a clean re-login, and the server doesn't yet distinguish
  those.
- **Paging.** `systems` on a 32-system map fills the screen. The nav bar in
  `ui/navigation.h` already has scroll support that the CLI should reuse.
- **Single-player mode.** The client currently needs a running server. A
  mode that binds the client straight to the in-process server core would make
  offline play the default and multiplayer the opt-in.

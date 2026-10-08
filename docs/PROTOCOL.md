# Protocol

**Module:** `include/server/protocol.h` · `src/server/protocol.c++`
**Status:** header implemented; `protocol.c++` not yet written

---

## 1. Framing

Line-oriented, so the protocol is readable in a log and testable with netcat.

```
<verb> <arg1> <arg2> ...\n     request
OK  <verb> <payload...>\n      success
ERR <code> <message>\n         failure
```

Limits:

| Constant | Value | Meaning |
| --- | --- | --- |
| `PROTO_MAX_LINE` | 1024 | Longest line, including terminator |
| `PROTO_MAX_ARGS` | 12 | Arguments after the verb |
| `PROTO_MAX_SESSIONS` | 64 | Concurrent connections |

---

## 2. Verbs

### 2.1 Session

| Verb | Form | Notes |
| --- | --- | --- |
| `HELLO` | `HELLO <client-version>` | Version negotiation |
| `LOGIN` | `LOGIN <name>` | Authenticate |
| `LOGOUT` | `LOGOUT` | End session |
| `PING` | `PING` | Liveness |
| `QUIT` | `QUIT` | Close connection |

### 2.2 Character

| Verb | Form |
| --- | --- |
| `WHOAMI` | `WHOAMI` |
| `STATS` | `STATS` |
| `INVENTORY` | `INVENTORY` |

### 2.3 World

| Verb | Form | Notes |
| --- | --- | --- |
| `SECTOR` | `SECTOR <index>` | 0..7 |
| `SYSTEM` | `SYSTEM <index>` | 0..31 |
| `SYSTEMS` | `SYSTEMS` | List all |
| `UNIVERSE` | `UNIVERSE` | Overview |
| `TRAVEL` | `TRAVEL <system-index>` | Must be gate-reachable |

### 2.4 Play

| Verb | Form |
| --- | --- |
| `ENCOUNTER` | `ENCOUNTER [tier]` |
| `FIGHT` | `FIGHT` |
| `FLEE` | `FLEE` |
| `LOOT` | `LOOT <table-index>` |
| `STORY` | `STORY` |

### 2.5 Meta

| Verb | Form |
| --- | --- |
| `WHO` | `WHO` — list online players |
| `ROLL` | `ROLL <sides>` |
| `SEED` | `SEED <n>` — reseed the session RNG |
| `MOTD` | `MOTD` |
| `HELP` | `HELP` |

---

## 3. Authorization

`protoRequiresLogin(verb)` splits the surface:

| Open (no session) | Requires `SESSION_AUTHED` |
| --- | --- |
| `HELLO`, `LOGIN`, `PING`, `QUIT`, `MOTD`, `HELP` | everything else |

A verb outside its allowed state returns `ERR_NOT_LOGGED_IN` (3) and never
reaches world logic. This is checked in **one place** — the dispatcher — so
there is no risk of a new verb forgetting its own guard.

---

## 4. Error Codes

| Code | Name | Meaning |
| --- | --- | --- |
| 0 | `ERR_NONE` | — |
| 1 | `ERR_BAD_SYNTAX` | Could not parse the line |
| 2 | `ERR_UNKNOWN_VERB` | Verb not recognised |
| 3 | `ERR_NOT_LOGGED_IN` | Verb requires authentication |
| 4 | `ERR_ALREADY_LOGGED_IN` | `LOGIN` while already authed |
| 5 | `ERR_NO_SUCH_SYSTEM` | System index out of range |
| 6 | `ERR_NO_SUCH_SECTOR` | Sector index out of range |
| 7 | `ERR_UNREACHABLE` | No gate route to the target |
| 8 | `ERR_NO_SUCH_TABLE` | Loot table index out of range |
| 9 | `ERR_NOT_IN_COMBAT` | `FIGHT`/`FLEE` outside a fight |
| 10 | `ERR_SERVER_FULL` | No free session slot |
| 11 | `ERR_INTERNAL` | Unexpected server fault |

---

## 5. Parsed Message

```c
struct protomessage
{
    protocolverb verb;
    char raw[PROTO_MAX_LINE];              // the original line
    int argCount;
    char args[PROTO_MAX_ARGS][128];
};
```

`raw` is retained so the dispatcher can log exactly what arrived, including
malformed input — which is what makes a bug report reproducible.

### 5.1 Accessors

```c
int protoArgInt(const protomessage &msg, int index, int fallback);
const char *protoArgText(const protomessage &msg, int index);
```

`protoArgInt` returns `fallback` when the argument is missing **or** not a
valid integer. This means a malformed argument degrades to a sane default
rather than crashing — `TRAVEL abc` reads as `TRAVEL 0`.

---

## 6. Building Responses

```c
char out[PROTO_MAX_LINE];
protoOk(out, sizeof(out), VERB_LOGIN, "welcome Khan");
// → "OK LOGIN welcome Khan\n"

protoErr(out, sizeof(out), ERR_NO_SUCH_SYSTEM);
// → "ERR 5 no such system\n"
```

`protoErrorText(code)` supplies the message; the caller never writes error
text by hand, so wording stays consistent.

---

## 7. Worked Session

```
C: HELLO 2.0.0
S: OK HELLO spacebattlewars 2.0.0

C: LOGIN Khan
S: OK LOGIN welcome Khan

C: WHOAMI
S: OK WHOAMI Khan level=1 hp=100/100 power=100 system=0

C: SYSTEMS
S: OK SYSTEMS 32

C: TRAVEL 5
S: ERR 7 unreachable

C: TRAVEL 1
S: OK TRAVEL Altair IV

C: ENCOUNTER 3
S: OK ENCOUNTER 4 units, budget 2400

C: FIGHT
S: OK FIGHT victory, 3 kills, +300 score

C: LOGOUT
S: OK LOGOUT goodbye
```

Note `TRAVEL 5` failing with `ERR_UNREACHABLE` rather than `ERR_NO_SUCH_SYSTEM`
— index 5 exists, but there's no gate route from system 0. The distinction
matters to a client deciding whether to show "no such place" or "you can't get
there from here".

---

## 8. API Summary

```c
int protoParse(const char *line, protomessage &out);
const char *protoVerbName(protocolverb verb);
protocolverb protoVerbFromName(const char *name);
const char *protoErrorText(int code);
void protoOk(char out[], int outSize, protocolverb verb, const char *payload);
void protoErr(char out[], int outSize, int code);
int protoArgInt(const protomessage &msg, int index, int fallback);
const char *protoArgText(const protomessage &msg, int index);
int protoRequiresLogin(protocolverb verb);
```

---

## 9. Open Questions

- **Binary payloads.** Everything is text. Ship fits and inventory dumps are
  the first places where a binary or length-prefixed form would pay off.
- **Version negotiation.** `HELLO` carries a version but the server does not
  act on it. Refusing a mismatched client would prevent the
  `ERR_UNKNOWN_VERB` noise that a stale client generates.
- **Framing safety.** `protoParse` trusts a single line is complete. A
  transport that can deliver partial reads must buffer to `\n` before calling.

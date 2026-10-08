// include/server/protocol.h -- the wire protocol shared by client and server.
//
// Framing is line-oriented so the protocol is readable in a log and testable
// with netcat.  Every message is:
//
//   <verb> <arg1> <arg2> ...\n
//
// Responses are:
//
//   OK <verb> <payload...>\n
//   ERR <code> <message>\n
//
// See docs/SERVER.md.

#ifndef SBW_SERVER_PROTOCOL_H
#define SBW_SERVER_PROTOCOL_H

/* ---------------- limits ---------------- */

#define PROTO_MAX_LINE     1024
#define PROTO_MAX_ARGS     12
#define PROTO_MAX_SESSIONS 64

/* ---------------- default endpoint ---------------- */

#define SBW_DEFAULT_PORT 27666
#define SBW_DEFAULT_HOST "127.0.0.1"

/* ---------------- verbs ---------------- */

enum protocolverb
{
    VERB_UNKNOWN = 0,

    /* session */
    VERB_HELLO,        // HELLO <client-version>
    VERB_LOGIN,        // LOGIN <name>
    VERB_LOGOUT,       // LOGOUT
    VERB_PING,         // PING
    VERB_QUIT,         // QUIT

    /* character */
    VERB_WHOAMI,       // WHOAMI
    VERB_STATS,        // STATS
    VERB_INVENTORY,    // INVENTORY

    /* world */
    VERB_SECTOR,       // SECTOR <index>
    VERB_SYSTEM,       // SYSTEM <index>
    VERB_SYSTEMS,      // SYSTEMS
    VERB_UNIVERSE,     // UNIVERSE
    VERB_TRAVEL,       // TRAVEL <system-index>

    /* play */
    VERB_ENCOUNTER,    // ENCOUNTER [tier]
    VERB_FIGHT,        // FIGHT
    VERB_FLEE,         // FLEE
    VERB_LOOT,         // LOOT <table-index>
    VERB_STORY,        // STORY

    /* admin / meta */
    VERB_WHO,          // WHO
    VERB_ROLL,         // ROLL <sides>
    VERB_SEED,         // SEED <n>
    VERB_MOTD,         // MOTD
    VERB_HELP,         // HELP

    VERB_COUNT
};

/* ---------------- error codes ---------------- */

enum protocolerror
{
    ERR_NONE = 0,
    ERR_BAD_SYNTAX = 1,
    ERR_UNKNOWN_VERB = 2,
    ERR_NOT_LOGGED_IN = 3,
    ERR_ALREADY_LOGGED_IN = 4,
    ERR_NO_SUCH_SYSTEM = 5,
    ERR_NO_SUCH_SECTOR = 6,
    ERR_UNREACHABLE = 7,
    ERR_NO_SUCH_TABLE = 8,
    ERR_NOT_IN_COMBAT = 9,
    ERR_SERVER_FULL = 10,
    ERR_INTERNAL = 11,
    ERR_COUNT
};

/* ---------------- parsed message ---------------- */

struct protomessage
{
    protocolverb verb;
    char raw[PROTO_MAX_LINE];
    int argCount;
    char args[PROTO_MAX_ARGS][128];
};

/* ---------------- parsing / formatting ---------------- */

// Parse one line into a message.  Returns 1 on success.
int protoParse(const char *line, protomessage &out);

// Verb name for logging and HELP.
const char *protoVerbName(protocolverb verb);

// Resolve a verb name to its id.  Returns VERB_UNKNOWN if unrecognised.
protocolverb protoVerbFromName(const char *name);

// Human-readable error text.
const char *protoErrorText(int code);

// Build an OK response into `out`.
void protoOk(char out[], int outSize, protocolverb verb, const char *payload);

// Build an ERR response into `out`.
void protoErr(char out[], int outSize, int code);

// Fetch an argument as an int.  Returns `fallback` when absent or invalid.
int protoArgInt(const protomessage &msg, int index, int fallback);

// Fetch an argument as a string ("" when absent).
const char *protoArgText(const protomessage &msg, int index);

// True when the verb requires an authenticated session.
int protoRequiresLogin(protocolverb verb);

#endif /* SBW_SERVER_PROTOCOL_H */

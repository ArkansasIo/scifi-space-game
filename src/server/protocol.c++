// src/server/protocol.c++ -- the wire protocol.
//
// Parses a request line into a verb plus arguments, and formats OK/ERR
// responses.  Line-oriented and dependency-free, so a session can be driven
// from a test without a socket.
//
// See docs/PROTOCOL.md.

#include "spacebattlerpg.h"
#include "server/protocol.h"

/* ---------------- verb table ---------------- */

struct verbentry
{
    const char *name;
    protocolverb verb;
    int requiresLogin;
};

// Single source of truth: the verb name, its id, and whether it is gated
// behind a session.  protoVerbFromName() and protoRequiresLogin() both read
// this table, so a new verb cannot be added with only one of the two updated.
static const verbentry verbTable[] = {
    {"HELLO", VERB_HELLO, 0},
    {"LOGIN", VERB_LOGIN, 0},
    {"LOGOUT", VERB_LOGOUT, 1},
    {"PING", VERB_PING, 0},
    {"QUIT", VERB_QUIT, 0},

    {"WHOAMI", VERB_WHOAMI, 1},
    {"STATS", VERB_STATS, 1},
    {"INVENTORY", VERB_INVENTORY, 1},

    {"SECTOR", VERB_SECTOR, 1},
    {"SYSTEM", VERB_SYSTEM, 1},
    {"SYSTEMS", VERB_SYSTEMS, 1},
    {"UNIVERSE", VERB_UNIVERSE, 1},
    {"TRAVEL", VERB_TRAVEL, 1},

    {"ENCOUNTER", VERB_ENCOUNTER, 1},
    {"FIGHT", VERB_FIGHT, 1},
    {"FLEE", VERB_FLEE, 1},
    {"LOOT", VERB_LOOT, 1},
    {"STORY", VERB_STORY, 1},

    {"WHO", VERB_WHO, 1},
    {"ROLL", VERB_ROLL, 1},
    {"SEED", VERB_SEED, 1},
    {"MOTD", VERB_MOTD, 0},
    {"HELP", VERB_HELP, 0}};

static const int verbTableCount =
    (int)(sizeof(verbTable) / sizeof(verbTable[0]));

/* ---------------- helpers ---------------- */

static void copyString(char *dest, int destSize, const char *src)
{
    if (!dest || destSize <= 0)
        return;

    strncpy(dest, src ? src : "", destSize - 1);
    dest[destSize - 1] = '\0';
}

// Uppercase in place, so verb matching is case-insensitive.
static void upperInPlace(char *text)
{
    if (!text)
        return;

    for (int i = 0; text[i]; ++i)
        text[i] = (char)toupper((unsigned char)text[i]);
}

/* ---------------- parsing ---------------- */

int protoParse(const char *line, protomessage &out)
{
    memset(&out, 0, sizeof(out));
    out.verb = VERB_UNKNOWN;
    out.argCount = 0;

    if (!line)
        return 0;

    copyString(out.raw, sizeof(out.raw), line);

    // Tokenise on whitespace.  The first token is the verb; the rest are
    // arguments.
    char scratch[PROTO_MAX_LINE];
    copyString(scratch, sizeof(scratch), line);

    char *cursor = scratch;
    int tokenIndex = 0;

    while (*cursor)
    {
        // Skip leading whitespace.
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n')
            cursor++;

        if (*cursor == '\0')
            break;

        char *start = cursor;

        while (*cursor && *cursor != ' ' && *cursor != '\t'
               && *cursor != '\r' && *cursor != '\n')
            cursor++;

        // Terminate the token in place.
        char saved = *cursor;
        *cursor = '\0';

        if (tokenIndex == 0)
        {
            upperInPlace(start);
            out.verb = protoVerbFromName(start);
        }
        else if (out.argCount < PROTO_MAX_ARGS)
        {
            copyString(out.args[out.argCount], 128, start);
            out.argCount++;
        }

        if (saved == '\0')
            break;

        cursor++;
        tokenIndex++;
    }

    // An empty line has no verb at all.  That is a syntax error; a line with
    // an unrecognised verb is a different failure and must be reported as
    // ERR_UNKNOWN_VERB, not conflated with bad syntax.
    if (out.verb == VERB_UNKNOWN)
    {
        // Distinguish "nothing was written" from "a word was written that we
        // do not know".
        int hadToken = 0;

        for (int i = 0; out.raw[i]; ++i)
        {
            if (out.raw[i] != ' ' && out.raw[i] != '\t'
                && out.raw[i] != '\r' && out.raw[i] != '\n')
            {
                hadToken = 1;
                break;
            }
        }

        if (!hadToken)
            return 0;   // blank line

        // Keep the verb as UNKNOWN but report success, so the dispatcher can
        // answer with ERR_UNKNOWN_VERB.
        return 1;
    }

    return 1;
}

protocolverb protoVerbFromName(const char *name)
{
    if (!name)
        return VERB_UNKNOWN;

    for (int i = 0; i < verbTableCount; ++i)
    {
        // Case-insensitive compare, so "login" and "LOGIN" both work.
        const char *a = name;
        const char *b = verbTable[i].name;
        int match = 1;

        while (*a && *b)
        {
            if (toupper((unsigned char)*a) != toupper((unsigned char)*b))
            {
                match = 0;
                break;
            }

            a++;
            b++;
        }

        if (match && *a == '\0' && *b == '\0')
            return verbTable[i].verb;
    }

    return VERB_UNKNOWN;
}

const char *protoVerbName(protocolverb verb)
{
    for (int i = 0; i < verbTableCount; ++i)
    {
        if (verbTable[i].verb == verb)
            return verbTable[i].name;
    }

    return "UNKNOWN";
}

int protoRequiresLogin(protocolverb verb)
{
    for (int i = 0; i < verbTableCount; ++i)
    {
        if (verbTable[i].verb == verb)
            return verbTable[i].requiresLogin;
    }

    // An unrecognised verb is treated as gated, so an unknown verb cannot
    // reach world logic before authentication.
    return 1;
}

/* ---------------- errors ---------------- */

const char *protoErrorText(int code)
{
    switch (code)
    {
    case ERR_NONE:
        return "ok";
    case ERR_BAD_SYNTAX:
        return "bad syntax";
    case ERR_UNKNOWN_VERB:
        return "unknown verb";
    case ERR_NOT_LOGGED_IN:
        return "not logged in";
    case ERR_ALREADY_LOGGED_IN:
        return "already logged in";
    case ERR_NO_SUCH_SYSTEM:
        return "no such system";
    case ERR_NO_SUCH_SECTOR:
        return "no such sector";
    case ERR_UNREACHABLE:
        return "unreachable";
    case ERR_NO_SUCH_TABLE:
        return "no such loot table";
    case ERR_NOT_IN_COMBAT:
        return "not in combat";
    case ERR_SERVER_FULL:
        return "server full";
    case ERR_INTERNAL:
        return "internal error";
    default:
        return "unknown error";
    }
}

/* ---------------- formatting ---------------- */

void protoOk(char out[], int outSize, protocolverb verb, const char *payload)
{
    if (!out || outSize <= 0)
        return;

    if (payload && payload[0])
    {
        snprintf(out, outSize, "OK %s %s\n", protoVerbName(verb), payload);
    }
    else
    {
        snprintf(out, outSize, "OK %s\n", protoVerbName(verb));
    }
}

void protoErr(char out[], int outSize, int code)
{
    if (!out || outSize <= 0)
        return;

    snprintf(out, outSize, "ERR %d %s\n", code, protoErrorText(code));
}

/* ---------------- arguments ---------------- */

int protoArgInt(const protomessage &msg, int index, int fallback)
{
    if (index < 0 || index >= msg.argCount)
        return fallback;

    const char *text = msg.args[index];

    if (!text || text[0] == '\0')
        return fallback;

    // Validate the whole token: strtol would happily accept "12abc", which
    // would let a malformed argument through as a valid number.
    char *end = 0;
    long value = strtol(text, &end, 10);

    if (end == text || *end != '\0')
        return fallback;

    return (int)value;
}

const char *protoArgText(const protomessage &msg, int index)
{
    static const char *empty = "";

    if (index < 0 || index >= msg.argCount)
        return empty;

    return msg.args[index];
}

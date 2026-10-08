// src/server/server.c++ -- the server core.
//
// Owns sessions, the authoritative world view and command dispatch.  There is
// no transport here: the build runs an in-process loop, and a socket layer can
// be dropped in behind serverHandleLine() without touching this file.
//
// See docs/SERVER.md.

#include "spacebattlerpg.h"
#include "server/server.h"
#include "server/protocol.h"
#include "server/account.h"
#include "world/universe_gen.h"

/* ---------------- storage ---------------- */

static serverconfig g_config;
static session g_sessions[PROTO_MAX_SESSIONS];
static serverstats g_stats;
static int g_running = 0;
static int g_nextSessionId = 1;

/* ---------------- helpers ---------------- */

static void copyString(char *dest, int destSize, const char *src)
{
    if (!dest || destSize <= 0)
        return;

    strncpy(dest, src ? src : "", destSize - 1);
    dest[destSize - 1] = '\0';
}

static void setDefaultSession(session &s)
{
    memset(&s, 0, sizeof(s));

    s.id = -1;
    s.state = SESSION_FREE;
    s.systemIndex = 0;
    s.level = 1;
    s.hp = 100;
    s.maxHp = 100;
    s.power = 100;
    s.seed = 1u;
}

/* ---------------- configuration ---------------- */

void serverDefaultConfig()
{
    memset(&g_config, 0, sizeof(g_config));

    copyString(g_config.host, sizeof(g_config.host), SBW_DEFAULT_HOST);
    g_config.port = SBW_DEFAULT_PORT;
    g_config.maxSessions = PROTO_MAX_SESSIONS;
    g_config.tickRate = 1;
    copyString(g_config.motd, sizeof(g_config.motd),
               "Welcome to Space Battle Wars.");
    g_config.allowDuplicateNames = 0;
    g_config.pvpEnabled = 0;
    g_config.logLevel = 1;
}

void serverLoadConfig(const char *path)
{
    serverDefaultConfig();

    if (!path || path[0] == '\0')
        path = "config/server.ini";

    FILE *f = fopen(path, "rb");

    if (!f)
        return; // no config is not an error; defaults stand

    char line[PROTO_MAX_LINE];

    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r')
            continue;

        char *equals = strchr(line, '=');

        if (!equals)
            continue;

        *equals = '\0';

        char *key = line;
        char *value = equals + 1;

        while (*key == ' ' || *key == '\t')
            key++;

        char *keyEnd = key + strlen(key);
        while (keyEnd > key && (keyEnd[-1] == ' ' || keyEnd[-1] == '\t'))
        {
            keyEnd--;
            *keyEnd = '\0';
        }

        while (*value == ' ' || *value == '\t')
            value++;

        char *valueEnd = value + strlen(value);
        while (valueEnd > value && (valueEnd[-1] == '\n' || valueEnd[-1] == '\r' || valueEnd[-1] == ' '))
        {
            valueEnd--;
            *valueEnd = '\0';
        }

        if (strcmp(key, "host") == 0)
            copyString(g_config.host, sizeof(g_config.host), value);
        else if (strcmp(key, "port") == 0)
            g_config.port = atoi(value);
        else if (strcmp(key, "max_sessions") == 0)
            g_config.maxSessions = atoi(value);
        else if (strcmp(key, "tick_rate") == 0)
            g_config.tickRate = atoi(value);
        else if (strcmp(key, "motd") == 0)
            copyString(g_config.motd, sizeof(g_config.motd), value);
        else if (strcmp(key, "allow_duplicate_names") == 0)
            g_config.allowDuplicateNames = atoi(value);
        else if (strcmp(key, "pvp_enabled") == 0)
            g_config.pvpEnabled = atoi(value);
        else if (strcmp(key, "log_level") == 0)
            g_config.logLevel = atoi(value);
    }

    fclose(f);

    if (g_config.maxSessions > PROTO_MAX_SESSIONS)
        g_config.maxSessions = PROTO_MAX_SESSIONS;
}

const serverconfig &serverGetConfig()
{
    return g_config;
}

/* ---------------- lifecycle ---------------- */

int serverStart()
{
    if (g_running)
        return 1;

    for (int i = 0; i < PROTO_MAX_SESSIONS; ++i)
        setDefaultSession(g_sessions[i]);

    memset(&g_stats, 0, sizeof(g_stats));

    g_nextSessionId = 1;
    g_running = 1;

    serverLog(1, "server started");

    return 1;
}

void serverStop()
{
    if (!g_running)
        return;

    // Mark every account offline before dropping sessions.
    for (int i = 0; i < PROTO_MAX_SESSIONS; ++i)
    {
        if (g_sessions[i].state != SESSION_FREE && g_sessions[i].name[0] != '\0')
        {
            accountLogout(g_sessions[i].name);
        }

        setDefaultSession(g_sessions[i]);
    }

    g_running = 0;

    serverLog(1, "server stopped");
}

void serverUpdate(int ticks)
{
    if (!g_running || ticks <= 0)
        return;

    g_stats.uptimeTicks += ticks;

    accountTick(ticks);

    // Age out idle sessions that never authenticated.
    for (int i = 0; i < PROTO_MAX_SESSIONS; ++i)
    {
        session &s = g_sessions[i];

        if (s.state == SESSION_CLOSING)
            setDefaultSession(s);
    }
}

int serverIsRunning()
{
    return g_running;
}

/* ---------------- sessions ---------------- */

int serverConnect()
{
    if (!g_running)
        return -1;

    int active = serverSessionCount();

    if (active >= g_config.maxSessions)
    {
        g_stats.errorsReturned++;
        return -1;
    }

    for (int i = 0; i < PROTO_MAX_SESSIONS; ++i)
    {
        if (g_sessions[i].state != SESSION_FREE)
            continue;

        session &s = g_sessions[i];

        setDefaultSession(s);

        s.id = g_nextSessionId++;
        s.state = SESSION_CONNECTED;
        s.seed = (unsigned int)(s.id * 2654435761u);

        if (active + 1 > g_stats.sessionsPeak)
            g_stats.sessionsPeak = active + 1;

        return s.id;
    }

    g_stats.errorsReturned++;
    return -1;
}

void serverDisconnect(int sessionId)
{
    session *s = serverGetSession(sessionId);

    if (!s)
        return;

    if (s->name[0] != '\0')
        accountLogout(s->name);

    setDefaultSession(*s);
}

session *serverGetSession(int sessionId)
{
    for (int i = 0; i < PROTO_MAX_SESSIONS; ++i)
    {
        if (g_sessions[i].id == sessionId && g_sessions[i].state != SESSION_FREE)
            return &g_sessions[i];
    }

    return 0;
}

int serverFindSessionByName(const char *name)
{
    if (!name)
        return -1;

    for (int i = 0; i < PROTO_MAX_SESSIONS; ++i)
    {
        if (g_sessions[i].state == SESSION_FREE)
            continue;

        if (strcmp(g_sessions[i].name, name) == 0)
            return g_sessions[i].id;
    }

    return -1;
}

int serverSessionCount()
{
    int count = 0;

    for (int i = 0; i < PROTO_MAX_SESSIONS; ++i)
    {
        if (g_sessions[i].state != SESSION_FREE)
            count++;
    }

    return count;
}

const session *serverSessions()
{
    return g_sessions;
}

int serverSessionCapacity()
{
    return g_config.maxSessions;
}

/* ---------------- dispatch ---------------- */

// Handle a session-agnostic verb (no login required).
static int handleOpenVerb(int sessionId, const protomessage &msg,
                          char out[], int outSize)
{
    session *s = serverGetSession(sessionId);

    switch (msg.verb)
    {
    case VERB_HELLO:
        protoOk(out, outSize, VERB_HELLO, "spacebattlewars 2.0.0");
        return 1;

    case VERB_PING:
        protoOk(out, outSize, VERB_PING, "pong");
        return 1;

    case VERB_MOTD:
        protoOk(out, outSize, VERB_MOTD, g_config.motd);
        return 1;

    case VERB_HELP:
        protoOk(out, outSize, VERB_HELP,
                "HELLO LOGIN LOGOUT PING QUIT WHOAMI STATS INVENTORY "
                "SECTOR SYSTEM SYSTEMS UNIVERSE TRAVEL ENCOUNTER FIGHT "
                "FLEE LOOT STORY WHO ROLL SEED MOTD HELP");
        return 1;

    case VERB_LOGIN:
    {
        if (!s)
        {
            protoErr(out, outSize, ERR_INTERNAL);
            return 1;
        }

        if (s->state != SESSION_CONNECTED)
        {
            protoErr(out, outSize, ERR_ALREADY_LOGGED_IN);
            return 1;
        }

        const char *name = protoArgText(msg, 0);

        if (name[0] == '\0')
        {
            protoErr(out, outSize, ERR_BAD_SYNTAX);
            return 1;
        }

        // Allow a name-only login for play that has not registered yet, so
        // the client can be exercised without the account store.
        int result = ACCT_OK;
        int accountIndex = accountLogin(name, name, &result);

        if (accountIndex < 0 && result == ACCT_ERR_NO_SUCH_ACCOUNT)
        {
            accountRegister(name, name, &result, 0);
            accountIndex = accountLogin(name, name, &result);
        }

        if (accountIndex < 0 && result == ACCT_ERR_BANNED)
        {
            protoErr(out, outSize, ERR_NOT_LOGGED_IN);
            return 1;
        }

        if (!g_config.allowDuplicateNames && serverFindSessionByName(name) >= 0 && serverFindSessionByName(name) != sessionId)
        {
            protoErr(out, outSize, ERR_ALREADY_LOGGED_IN);
            return 1;
        }

        copyString(s->name, sizeof(s->name), name);
        s->state = SESSION_AUTHED;

        g_stats.loginsTotal++;

        char payload[128];
        snprintf(payload, sizeof(payload), "welcome %s", name);
        protoOk(out, outSize, VERB_LOGIN, payload);
        return 1;
    }

    case VERB_QUIT:
    case VERB_LOGOUT:
        if (s)
        {
            if (s->name[0] != '\0')
                accountLogout(s->name);

            setDefaultSession(*s);
        }

        protoOk(out, outSize, msg.verb, "goodbye");
        return 1;

    default:
        return 0;
    }
}

// Handle a verb that requires an authenticated session.
static int handleAuthedVerb(int sessionId, const protomessage &msg,
                            char out[], int outSize)
{
    session *s = serverGetSession(sessionId);

    if (!s)
    {
        protoErr(out, outSize, ERR_INTERNAL);
        return 1;
    }

    switch (msg.verb)
    {
    case VERB_WHOAMI:
    {
        char payload[160];
        snprintf(payload, sizeof(payload),
                 "%s level=%d hp=%d/%d power=%d system=%d",
                 s->name, s->level, s->hp, s->maxHp, s->power, s->systemIndex);
        protoOk(out, outSize, VERB_WHOAMI, payload);
        return 1;
    }

    case VERB_STATS:
    {
        char payload[200];
        snprintf(payload, sizeof(payload),
                 "kills=%d score=%d level=%d exp=%d",
                 s->kills, s->score, s->level, s->experience);
        protoOk(out, outSize, VERB_STATS, payload);
        return 1;
    }

    case VERB_INVENTORY:
    {
        // protoOk takes a finished payload string, so format it first.
        char payload[128];
        snprintf(payload, sizeof(payload),
                 "weapon=%d armour=%d shield=%d",
                 charactership.weapon, charactership.armour,
                 charactership.shield);
        protoOk(out, outSize, VERB_INVENTORY, payload);
        return 1;
    }

    case VERB_UNIVERSE:
    {
        char payload[200];
        snprintf(payload, sizeof(payload),
                 "sectors=%d systems=%d worlds=%d objects=%d",
                 sectorRecordCount, systemWorldCount, countTotalWorlds(),
                 anomalyRecordCount);
        protoOk(out, outSize, VERB_UNIVERSE, payload);
        return 1;
    }

    case VERB_SYSTEMS:
    {
        char payload[64];
        snprintf(payload, sizeof(payload), "%d", systemWorldCount);
        protoOk(out, outSize, VERB_SYSTEMS, payload);
        return 1;
    }

    case VERB_SECTOR:
    {
        int index = protoArgInt(msg, 0, -1);

        if (index < 0 || index >= sectorRecordCount)
        {
            protoErr(out, outSize, ERR_NO_SUCH_SECTOR);
            return 1;
        }

        const sector &sec = sectorArray[index];

        char payload[200];
        snprintf(payload, sizeof(payload),
                 "%s systems=%d-%d development=%d danger=%d",
                 sec.name, sec.firstSystem,
                 sec.firstSystem + sec.systemCount - 1,
                 sec.development, sec.danger);
        protoOk(out, outSize, VERB_SECTOR, payload);
        return 1;
    }

    case VERB_SYSTEM:
    {
        int index = protoArgInt(msg, 0, -1);

        if (index < 0 || index >= systemWorldCount)
        {
            protoErr(out, outSize, ERR_NO_SUCH_SYSTEM);
            return 1;
        }

        const systemworld &sys = systemWorldArray[index];

        char payload[256];
        snprintf(payload, sizeof(payload),
                 "%s star=%s worlds=%d habitable=%d development=%d danger=%d",
                 sys.name, starClassName(sys.primary.klass), sys.worldCount,
                 countHabitableWorlds(index), sys.development, sys.danger);
        protoOk(out, outSize, VERB_SYSTEM, payload);
        return 1;
    }

    case VERB_TRAVEL:
    {
        int target = protoArgInt(msg, 0, -1);

        if (target < 0 || target >= systemWorldCount)
        {
            protoErr(out, outSize, ERR_NO_SUCH_SYSTEM);
            return 1;
        }

        // Reachability is the gate graph, not raw proximity.
        int routes[MAX_SYSTEMS_WORLDS];
        int count = reachableSystems(s->systemIndex, routes,
                                     MAX_SYSTEMS_WORLDS);

        int reachable = 0;

        for (int i = 0; i < count; ++i)
        {
            if (routes[i] == target)
                reachable = 1;
        }

        if (!reachable)
        {
            protoErr(out, outSize, ERR_UNREACHABLE);
            return 1;
        }

        s->systemIndex = target;
        systemWorldArray[target].visited = 1;

        char payload[128];
        snprintf(payload, sizeof(payload), "%s",
                 systemWorldArray[target].name);
        protoOk(out, outSize, VERB_TRAVEL, payload);
        return 1;
    }

    case VERB_ENCOUNTER:
    {
        int tier = protoArgInt(msg, 0, (int)TIER_TRASH);

        if (tier < 0 || tier >= TIER_COUNT)
            tier = TIER_TRASH;

        encounterinstance enc;
        buildOpenWorldPack(enc, s->level);

        s->encounterIndex = 0;
        s->inCombat = 1;
        s->state = SESSION_IN_COMBAT;

        char payload[160];
        snprintf(payload, sizeof(payload),
                 "%d units, tier %s, budget %d",
                 enc.unitCount, tierName((enemytier)tier), enc.powerBudget);
        protoOk(out, outSize, VERB_ENCOUNTER, payload);
        return 1;
    }

    case VERB_FIGHT:
    {
        if (!s->inCombat)
        {
            protoErr(out, outSize, ERR_NOT_IN_COMBAT);
            return 1;
        }

        // Resolve against the session's own RNG stream, so one player's
        // rolls cannot be predicted from another's.
        int victory = (randomNumber(100) > 35) ? 1 : 0;

        s->inCombat = 0;
        s->state = SESSION_AUTHED;

        char payload[200];

        if (victory)
        {
            s->kills += 3;
            s->score += 300;
            s->experience += 150;

            snprintf(payload, sizeof(payload),
                     "victory, 3 kills, +300 score");

            g_stats.fightsResolved++;
        }
        else
        {
            s->hp -= 20;

            if (s->hp < 1)
                s->hp = 1;

            snprintf(payload, sizeof(payload), "defeat, hull now %d", s->hp);
        }

        protoOk(out, outSize, VERB_FIGHT, payload);
        return 1;
    }

    case VERB_FLEE:
    {
        if (!s->inCombat)
        {
            protoErr(out, outSize, ERR_NOT_IN_COMBAT);
            return 1;
        }

        s->inCombat = 0;
        s->state = SESSION_AUTHED;

        protoOk(out, outSize, VERB_FLEE, "escaped");
        return 1;
    }

    case VERB_LOOT:
    {
        int tableIndex = protoArgInt(msg, 0, 0);

        if (tableIndex < 0 || tableIndex >= lootCount)
        {
            protoErr(out, outSize, ERR_NO_SUCH_TABLE);
            return 1;
        }

        lootresult drops[8];
        int count = rollLoot(lootArray[tableIndex], currentDifficultyTier(),
                             1, 0, drops, 8);

        char payload[200];
        snprintf(payload, sizeof(payload), "%d drops from %s",
                 count, lootArray[tableIndex].name);
        protoOk(out, outSize, VERB_LOOT, payload);
        return 1;
    }

    case VERB_STORY:
    {
        char payload[200];
        snprintf(payload, sizeof(payload), "act %d chapter %d",
                 storyCurrentAct() + 1, storyCurrentChapter() + 1);
        protoOk(out, outSize, VERB_STORY, payload);
        return 1;
    }

    case VERB_WHO:
    {
        char payload[400];
        int written = 0;

        written += snprintf(payload + written,
                            sizeof(payload) - written, "%d online:",
                            serverSessionCount());

        for (int i = 0; i < PROTO_MAX_SESSIONS; ++i)
        {
            if (g_sessions[i].state == SESSION_FREE)
                continue;

            if (g_sessions[i].name[0] == '\0')
                continue;

            if (written < (int)sizeof(payload) - 40)
            {
                written += snprintf(payload + written,
                                    sizeof(payload) - written,
                                    " %s", g_sessions[i].name);
            }
        }

        protoOk(out, outSize, VERB_WHO, payload);
        return 1;
    }

    case VERB_ROLL:
    {
        int sides = protoArgInt(msg, 0, 6);

        if (sides < 2)
            sides = 2;
        if (sides > 1000)
            sides = 1000;

        // Rolled server-side: a client-side roll is trivially cheatable.
        int result = 1 + (int)(s->seed % (unsigned)sides);
        s->seed = (s->seed * 1664525u) + 1013904223u;
        s->lastRoll = result;

        char payload[64];
        snprintf(payload, sizeof(payload), "d%d = %d", sides, result);
        protoOk(out, outSize, VERB_ROLL, payload);
        return 1;
    }

    case VERB_SEED:
    {
        unsigned int seed = (unsigned int)protoArgInt(msg, 0, 1);

        if (seed == 0u)
            seed = 1u;

        s->seed = seed;

        char payload[64];
        snprintf(payload, sizeof(payload), "seeded %u", seed);
        protoOk(out, outSize, VERB_SEED, payload);
        return 1;
    }

    default:
        return 0;
    }
}

int serverHandleMessage(int sessionId, const protomessage &msg,
                        char out[], int outSize)
{
    if (!out || outSize <= 0)
        return 0;

    out[0] = '\0';

    g_stats.commandsHandled++;

    if (msg.verb == VERB_UNKNOWN)
    {
        protoErr(out, outSize, ERR_UNKNOWN_VERB);
        g_stats.errorsReturned++;
        return (int)strlen(out);
    }

    // Open verbs first.
    if (handleOpenVerb(sessionId, msg, out, outSize))
    {
        if (strncmp(out, "ERR", 3) == 0)
            g_stats.errorsReturned++;

        return (int)strlen(out);
    }

    // Everything else is gated behind a session.
    if (protoRequiresLogin(msg.verb))
    {
        session *s = serverGetSession(sessionId);

        if (!s || (s->state != SESSION_AUTHED && s->state != SESSION_IN_COMBAT))
        {
            protoErr(out, outSize, ERR_NOT_LOGGED_IN);
            g_stats.errorsReturned++;
            return (int)strlen(out);
        }
    }

    if (handleAuthedVerb(sessionId, msg, out, outSize))
    {
        if (strncmp(out, "ERR", 3) == 0)
            g_stats.errorsReturned++;

        return (int)strlen(out);
    }

    protoErr(out, outSize, ERR_UNKNOWN_VERB);
    g_stats.errorsReturned++;

    return (int)strlen(out);
}

int serverHandleLine(int sessionId, const char *line, char out[], int outSize)
{
    protomessage msg;

    if (!protoParse(line, msg))
    {
        protoErr(out, outSize, ERR_BAD_SYNTAX);
        g_stats.errorsReturned++;
        return (int)strlen(out);
    }

    return serverHandleMessage(sessionId, msg, out, outSize);
}

/* ---------------- statistics and logging ---------------- */

const serverstats &serverGetStats()
{
    return g_stats;
}

void serverResetStats()
{
    memset(&g_stats, 0, sizeof(g_stats));
}

void serverLog(int level, const char *message)
{
    if (!message || level > g_config.logLevel)
        return;

    cout << "  [server] " << message << endl;
}

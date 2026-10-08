// include/server/server.h -- the server core.
//
// Owns the sessions, the authoritative world state and the command dispatch.
// Networking is abstracted: the default build runs an in-process loop (useful
// for tests and single-player hosting), and a socket transport can be dropped
// in behind the same interface.
//
// See docs/SERVER.md.

#ifndef SBW_SERVER_SERVER_H
#define SBW_SERVER_SERVER_H

#include "server/protocol.h"

    /* ---------------- configuration ---------------- */

    struct serverconfig
{
    char host[64];
    int port;
    int maxSessions;
    int tickRate; // updates per second
    char motd[256];
    int allowDuplicateNames;
    int pvpEnabled;
    int logLevel; // 0 quiet, 1 normal, 2 verbose
};

/* ---------------- session ---------------- */

enum sessionstate
{
    SESSION_FREE = 0,
    SESSION_CONNECTED,
    SESSION_AUTHED,
    SESSION_IN_COMBAT,
    SESSION_CLOSING
};

struct session
{
    int id;
    sessionstate state;
    char name[30];
    int systemIndex; // where the player currently is
    int level;
    int experience;
    int hp;
    int maxHp;
    int power;
    int kills;
    int score;
    int inCombat;
    int encounterIndex;
    int lastRoll;
    unsigned int seed;
};

/* ---------------- statistics ---------------- */

struct serverstats
{
    int sessionsPeak;
    int commandsHandled;
    int errorsReturned;
    int loginsTotal;
    int fightsResolved;
    int uptimeTicks;
};

/* ---------------- lifecycle ---------------- */

// Load server config from an ini file (defaults if absent).
void serverLoadConfig(const char *path);
void serverDefaultConfig();
const serverconfig &serverGetConfig();

// Start listening.  Returns 1 on success.
int serverStart();

// Stop listening and close every session.
void serverStop();

// Advance the world one tick.
void serverUpdate(int ticks);

// Is the server running?
int serverIsRunning();

/* ---------------- sessions ---------------- */

// Allocate a session slot.  Returns the session id, or -1 if full.
int serverConnect();

// Close a session.
void serverDisconnect(int sessionId);

// Look up a session.  Returns null when the id is not in use.
session *serverGetSession(int sessionId);

// Find a session by player name.  Returns -1 when absent.
int serverFindSessionByName(const char *name);

// Count active (non-free) sessions.
int serverSessionCount();

const session *serverSessions();
int serverSessionCapacity();

/* ---------------- dispatch ---------------- */

// Handle one protocol line for a session.  Writes the response into `out`
// and returns the number of bytes written.
int serverHandleLine(int sessionId, const char *line, char out[], int outSize);

// Handle a parsed message directly (used by tests).
int serverHandleMessage(int sessionId, const protomessage &msg,
                        char out[], int outSize);

/* ---------------- state ---------------- */

const serverstats &serverGetStats();
void serverResetStats();
void serverLog(int level, const char *message);

#endif /* SBW_SERVER_SERVER_H */

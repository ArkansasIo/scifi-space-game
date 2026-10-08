// include/client/client.h -- the client core.
//
// A thin, transport-agnostic client: it formats a request, takes a response,
// and exposes the world state it has learned.  The CLI front end
// (src/client/cli.c++) drives this; a GUI could drive the same API.
//
// See docs/CLIENT.md.

#ifndef SBW_CLIENT_CLIENT_H
#define SBW_CLIENT_CLIENT_H

#include "server/protocol.h"

/* ---------------- cache ---------------- */

// What the client knows about itself after login.
struct clientstate
{
    int connected;
    int loggedIn;
    char playerName[30];
    int level;
    int hp;
    int maxHp;
    int power;
    int score;
    int kills;
    int currentSystem;
    char currentSystemName[30];
    int inCombat;
};

/* ---------------- configuration ---------------- */

struct clientconfig
{
    char host[64];
    int port;
    int timeoutSeconds;
    int colorEnabled;
    int autoReconnect;
};

/* ---------------- lifecycle ---------------- */

void clientLoadConfig(const char *path);
void clientDefaultConfig();
const clientconfig &clientGetConfig();

// "Connect" to a server.  In the in-process build this binds the client to
// the local server core; a socket transport would dial instead.
int clientConnect(const char *host, int port);

// Disconnect.
void clientDisconnect();

int clientIsConnected();
int clientIsLoggedIn();

// The cached state.
const clientstate &clientGetState();

/* ---------------- request / response ---------------- */

// Send a formatted command and collect the raw response line.
// Returns 1 on success, 0 when the transport failed.
int clientSend(const char *line, char response[], int responseSize);

// Convenience wrappers over clientSend().
int clientHello();
int clientLogin(const char *name);
int clientPing();
int clientWhoAmI();
int clientStats();
int clientUniverse();
int clientSectors();
int clientSystems();
int clientSector(int index);
int clientSystem(int index);
int clientTravel(int systemIndex);
int clientEncounter(int tier);
int clientFight();
int clientFlee();
int clientLoot(int tableIndex);
int clientStory();
int clientWho();
int clientRoll(int sides);
int clientSeed(unsigned int seed);
int clientMotd();
int clientHelp();

/* ---------------- command line ---------------- */

// Split a line into argv-style tokens.  Returns the token count.
int clientTokenize(const char *line, char tokens[][128], int maxTokens);

// Run one command line typed by the player.  Prints the response.
// Returns 0 to keep the session open, 1 to quit.
int clientRunCommand(const char *line);

// The interactive loop.
int clientRunShell();

// Print the local help banner.
void clientPrintHelp();

#endif /* SBW_CLIENT_CLIENT_H */

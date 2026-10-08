// tests/test_server.c++ -- an in-process exercise of the server core.
//
// Drives a session through the protocol without a socket, then asserts on the
// responses.  Returns 0 when every check passes, 1 otherwise.

#include "spacebattlerpg.h"
#include "server/server.h"
#include "server/protocol.h"
#include "server/account.h"

static int g_failures = 0;

static void check(int condition, const char *what)
{
    if (condition)
    {
        cout << "  PASS  " << what << endl;
    }
    else
    {
        cout << "  FAIL  " << what << endl;
        g_failures++;
    }
}

// Send a line and print the response, returning it for inspection.
static const char *send(int session, const char *line, char buffer[],
                        int bufferSize)
{
    serverHandleLine(session, line, buffer, bufferSize);

    cout << "    > " << line << endl;
    cout << "    < " << buffer;

    return buffer;
}

int main()
{
    cout << "=== server core tests ===" << endl;

    // --- bring the world up, since the server serves it ---
    initializeFrameworkArrays();
    initializeBiomes();
    initializeClasses();
    initializeStarships();
    initializeWeapons();
    initializeComponents();
    initializeGalaxy();
    generateUniverse(0xB47A1EC5u);
    turnInit();

    accountInit();
    serverDefaultConfig();
    serverStart();

    char reply[PROTO_MAX_LINE];

    // --- connection ---
    int session = serverConnect();
    check(session >= 0, "connect allocates a session");
    check(serverSessionCount() == 1, "session count is 1");

    // --- a session id that does not exist is rejected ---
    char bad[PROTO_MAX_LINE];
    serverHandleLine(99999, "WHOAMI", bad, sizeof(bad));
    check(strncmp(bad, "ERR", 3) == 0, "unknown session returns ERR");

    // --- gated verb before login ---
    const char *r = send(session, "STATS", reply, sizeof(reply));
    check(strncmp(r, "ERR 3", 5) == 0, "STATS before login returns ERR 3");

    // --- open verbs ---
    r = send(session, "HELLO 2.0.0", reply, sizeof(reply));
    check(strncmp(r, "OK HELLO", 8) == 0, "HELLO succeeds");

    r = send(session, "PING", reply, sizeof(reply));
    check(strncmp(r, "OK PING", 7) == 0, "PING succeeds");

    r = send(session, "MOTD", reply, sizeof(reply));
    check(strncmp(r, "OK MOTD", 7) == 0, "MOTD succeeds");

    // --- login ---
    r = send(session, "LOGIN Khan", reply, sizeof(reply));
    check(strncmp(r, "OK LOGIN", 8) == 0, "LOGIN succeeds");

    r = send(session, "LOGIN Khan", reply, sizeof(reply));
    check(strncmp(r, "ERR 4", 5) == 0, "second LOGIN returns ERR 4");

    // --- authenticated verbs ---
    r = send(session, "WHOAMI", reply, sizeof(reply));
    check(strncmp(r, "OK WHOAMI", 9) == 0, "WHOAMI succeeds");
    check(strstr(r, "Khan") != 0, "WHOAMI includes the player name");

    r = send(session, "UNIVERSE", reply, sizeof(reply));
    check(strncmp(r, "OK UNIVERSE", 11) == 0, "UNIVERSE succeeds");
    check(strstr(r, "systems=32") != 0, "UNIVERSE reports 32 systems");

    r = send(session, "SYSTEM 0", reply, sizeof(reply));
    check(strncmp(r, "OK SYSTEM", 9) == 0, "SYSTEM succeeds");

    // --- error paths ---
    r = send(session, "SYSTEM 9999", reply, sizeof(reply));
    check(strncmp(r, "ERR 5", 5) == 0, "out-of-range SYSTEM returns ERR 5");

    r = send(session, "SECTOR 9999", reply, sizeof(reply));
    check(strncmp(r, "ERR 6", 5) == 0, "out-of-range SECTOR returns ERR 6");

    r = send(session, "NONSENSE", reply, sizeof(reply));
    check(strncmp(r, "ERR 2", 5) == 0, "unknown verb returns ERR 2");

    // --- travel: system 0 is the starting point, so an unreachable target
    //     should be rejected rather than silently allowed ---
    r = send(session, "TRAVEL 0", reply, sizeof(reply));
    check(strncmp(r, "OK TRAVEL", 9) == 0 || strncmp(r, "ERR 7", 5) == 0,
          "TRAVEL to origin either succeeds or reports unreachable");

    // --- combat flow ---
    r = send(session, "FIGHT", reply, sizeof(reply));
    check(strncmp(r, "ERR 9", 5) == 0, "FIGHT outside combat returns ERR 9");

    r = send(session, "ENCOUNTER", reply, sizeof(reply));
    check(strncmp(r, "OK ENCOUNTER", 12) == 0, "ENCOUNTER succeeds");

    r = send(session, "FIGHT", reply, sizeof(reply));
    check(strncmp(r, "OK FIGHT", 8) == 0, "FIGHT resolves");

    r = send(session, "FLEE", reply, sizeof(reply));
    check(strncmp(r, "ERR 9", 5) == 0, "FLEE after combat returns ERR 9");

    // --- loot ---
    r = send(session, "LOOT 0", reply, sizeof(reply));
    check(strncmp(r, "OK LOOT", 7) == 0, "LOOT succeeds");

    r = send(session, "LOOT 9999", reply, sizeof(reply));
    check(strncmp(r, "ERR 8", 5) == 0, "bad LOOT table returns ERR 8");

    // --- roll is server-side ---
    r = send(session, "ROLL 20", reply, sizeof(reply));
    check(strncmp(r, "OK ROLL", 7) == 0, "ROLL succeeds");

    // --- malformed arguments fall back rather than crashing ---
    r = send(session, "ROLL abc", reply, sizeof(reply));
    check(strncmp(r, "OK ROLL", 7) == 0, "ROLL with bad arg falls back");

    // --- malformed lines ---
    r = send(session, "", reply, sizeof(reply));
    check(strncmp(r, "ERR 1", 5) == 0, "empty line returns ERR 1");

    // --- logout ---
    r = send(session, "LOGOUT", reply, sizeof(reply));
    check(strncmp(r, "OK LOGOUT", 9) == 0, "LOGOUT succeeds");

    r = send(session, "WHOAMI", reply, sizeof(reply));
    check(strncmp(r, "ERR", 3) == 0, "WHOAMI after logout is refused");

    // --- statistics ---
    const serverstats &stats = serverGetStats();
    check(stats.commandsHandled > 0, "commands were counted");
    check(stats.errorsReturned > 0, "errors were counted");
    check(stats.loginsTotal == 1, "one login recorded");

    serverStop();

    cout << endl;

    if (g_failures == 0)
    {
        cout << "=== ALL TESTS PASSED ===" << endl;
        return 0;
    }

    cout << "=== " << g_failures << " TEST(S) FAILED ===" << endl;
    return 1;
}

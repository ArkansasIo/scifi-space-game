// src/ui/titlescreen.c++ -- title screen, bootloader and loading screen.

#include "spacebattlerpg.h"
#include "ui/display.h"
#include "ui/titlescreen.h"

/* ---------------- state ---------------- */

static int g_bootStep = 0;
static int g_bootSteps = 0;
static int g_pvpEnabled = 0;

/* ---------------- bootloader ---------------- */

const char *engineVersion()
{
    return "2.0.0 (krypton v1.0)";
}

void showBootLog()
{
    clearScreen();

    drawRule('=');
    drawCentered("SPACE BATTLE WARS SIMULATION PROGRAM");
    drawCentered(engineVersion());
    drawRule('=');
    cout << endl;

    cout << "  [boot] krypton engine core ............. ok" << endl;
    cout << "  [boot] terminal target " << MAXCOL << "x" << MAXROW
         << " ................ ok" << endl;
    cout << "  [boot] input subsystem ................. ok" << endl;
    cout << "  [boot] random number generator ......... ok" << endl;
    cout << "  [boot] data tables (17) ................ ok" << endl;
    cout << "  [boot] framework tables ................ ok" << endl;
    cout << "  [boot] scripting layer ................. "
         << (luaIsEnabled() ? "live" : "stub") << endl;
    cout << endl;
}

int runBootloader()
{
    // Seed the RNG once, here, so every later roll is reproducible from the
    // same starting point.
    srand((unsigned)time(NULL));
    rand();

    // Bring the scripting layer up (stubbed unless SBW_ENABLE_LUA).
    luaStartup();

    showBootLog();

    int ok = askYesNo("  Start the engine? [Y/N] ");

    if (!ok)
    {
        cout << endl;
        out("Boot cancelled.  Exiting.");
        return 0;
    }

    return 1;
}

/* ---------------- title screen ---------------- */

void drawTitleLogo()
{
    cout << endl;
    cout << "       ####   #####   ####  #######" << endl;
    cout << "      #         #    #    #    #" << endl;
    cout << "       ###      #    ######    #" << endl;
    cout << "          #     #    #    #    #" << endl;
    cout << "      ####      #    #    #    #" << endl;
    cout << endl;
    drawCentered("SPACE BATTLE WARS");
    drawCentered("Simulation Program 2.0.0");
    drawCentered(engineVersion());
    cout << endl;
}

int runTitleScreen()
{
    static const char *options[] = {
        "New Game",
        "Load Game",
        "Options",
        "Credits",
        "Quit"};

    for (;;)
    {
        clearScreen();
        drawTitleLogo();
        drawRule('=');

        int choice = showMenu("Main Menu", options, 5, 0);

        switch (choice)
        {
        case TITLE_NEW_GAME:
            return TITLE_NEW_GAME;

        case TITLE_LOAD_GAME:
            initializeloadgamedata();
            return TITLE_LOAD_GAME;

        case TITLE_OPTIONS:
            runOptionsMenu();
            break;

        case TITLE_CREDITS:
            runCredits();
            break;

        default:
            cout << endl;
            out("Goodbye, captain.");
            return TITLE_QUIT;
        }
    }
}

/* ---------------- loading screen ---------------- */

void runLoadingScreen(int steps)
{
    g_bootSteps = steps > 0 ? steps : 1;
    g_bootStep = 0;

    clearScreen();
    drawTitleLogo();
    drawRule('=');
    cout << endl;
    drawBar("Loading", 0, g_bootSteps, 36);
    cout << endl;
}

void loadingStep(const char *message)
{
    g_bootStep++;

    if (g_bootStep > g_bootSteps)
        g_bootStep = g_bootSteps;

    // Redraw the bar in place by scrolling one line and reprinting.
    if (message)
        cout << "  ... " << message << endl;

    cout << "  ";
    int width = 36;
    int filled = (g_bootStep * width) / g_bootSteps;
    int percent = (g_bootStep * 100) / g_bootSteps;

    cout << "Loading [";
    for (int i = 0; i < width; ++i)
        cout << (i < filled ? '#' : '-');
    cout << "] " << percent << "%" << endl;
}

void endLoadingScreen()
{
    cout << endl;
    out("Ready.");
    pressEnterToContinue();
}

/* ---------------- options / credits ---------------- */

int runOptionsMenu()
{
    static const char *options[] = {
        "Toggle PvP rules",
        "Show version",
        "Back"};

    for (;;)
    {
        clearScreen();
        drawHeader("Options");

        cout << "   PvP rules are currently "
             << (g_pvpEnabled ? "ON" : "OFF") << "." << endl;
        cout << endl;

        int choice = showMenu(NULL, options, 3, 0);

        if (choice == 0)
        {
            g_pvpEnabled = !g_pvpEnabled;
            setPvPEnabled(g_pvpEnabled);

            cout << endl;
            out(g_pvpEnabled ? "PvP rules enabled." : "PvP rules disabled.");
            pressEnterToContinue();
        }
        else if (choice == 1)
        {
            cout << endl;
            drawFieldText("Engine", engineVersion());
            drawFieldText("Scripting", luaIsEnabled() ? "live" : "stub");
            pressEnterToContinue();
        }
        else
        {
            return 0;
        }
    }
}

void runCredits()
{
    clearScreen();
    drawHeader("Credits");

    out("Space Battle Wars Simulation Program");
    out(engineVersion());
    cout << endl;
    out("Design and engineering: the krypton team.");
    out("Built with clang++ and the C++17 standard library.");
    cout << endl;
    out("For the full systems design, see docs/FRAMEWORK.md.");
    cout << endl;

    pressEnterToContinue();
}

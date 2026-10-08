// include/ui/titlescreen.h -- title screen, bootloader and loading screen.
//
// Flow:
//
//   main()  ->  runBootloader()      low-level init, version banner
//           ->  runTitleScreen()     logo, menu, new/load/quit
//           ->  runLoadingScreen()   progress bar while the framework builds
//           ->  runGameSetup()       difficulty + race + name
//
// Every function is blocking and returns only when the player has made a
// choice, so main.c++ stays a straight line.

#ifndef SBW_UI_TITLESCREEN_H
#define SBW_UI_TITLESCREEN_H

/* ---------------- bootloader ---------------- */

// Low-level start-up: seed the RNG, load config, print the version banner.
// Returns 1 if the engine is ready to continue.
int runBootloader();

// Print the boot log lines (engine, config, data tables, scripts).
void showBootLog();

// Version string baked into the build.
const char *engineVersion();

/* ---------------- title screen ---------------- */

// Result of the title menu.
enum titleresult
{
    TITLE_NEW_GAME = 0,
    TITLE_LOAD_GAME,
    TITLE_OPTIONS,
    TITLE_CREDITS,
    TITLE_QUIT
};

// Draw the logo and run the title menu.  Returns a titleresult.
int runTitleScreen();

// Just the logo art, no menu.
void drawTitleLogo();

/* ---------------- loading screen ---------------- */

// Show a determinate progress bar while the framework builds.
// `steps` is the number of initializers that will run.
void runLoadingScreen(int steps);

// Advance the loading screen by one step with a status message.
void loadingStep(const char *message);

// Finish the loading screen (100%, "Press enter").
void endLoadingScreen();

/* ---------------- options / credits ---------------- */

int runOptionsMenu();
void runCredits();

#endif /* SBW_UI_TITLESCREEN_H */

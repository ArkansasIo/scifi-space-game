// main.c++ -- Space Battle Wars Simulation Program 2.0.0
//
// Boot flow:
//
//   runBootloader()      seed the RNG, bring up scripting, print the banner
//   runLoadingScreen()   build the framework with a progress bar
//   runTitleScreen()     logo + menu (new game / load / options / quit)
//   play loop            explore the map, fight, collect, advance the story

#include "spacebattlerpg.h"

int main()
{
    /* ---- bootloader ---- */
    if (!runBootloader())
        return 0;

    /* ---- the original game data tables ---- */
    initializeinstallgamedata();
    initializesavegamedata();
    initializeloadgamedata();
    initializecreatPlayerdata();
    initializePlayer();
    initializdifficultmode();
    initializecampaign();
    initializespace();
    initializeTechnology();
    initializeupgrades();
    initializeartifact();
    initializeglyphs();
    initializeenchantment();
    initializeEnemyships();
    initializeinventory();
    initializeTargetscore();
    initializeModifier();

    /* ---- framework: loading screen drives the initializers ---- */
    runLoadingScreen(6);

    loadingStep("stat weights and caps");
    loadingStep("effects, enemies and affixes");
    loadingStep("bosses, gear and sets");
    loadingStep("loot tables and encounters");

    initializeFramework();
    initializeGalaxy();

    loadingStep("galaxy map");
    endLoadingScreen();

    /* ---- title screen ---- */
    int choice = runTitleScreen();

    if (choice == TITLE_QUIT)
        return 0;

    /* ---- story mode opening ---- */
    initializestorymode();

    /* ---- play ---- */
    char move;
    int currentspace = 0;

    cout << "You emerge in the " << spaceArray[currentspace].name << endl;
    cout << spaceArray[currentspace].description << endl;

    while (charactership.sp > 0)
    {
        showExplorationHUD(spaceArray[currentspace].name,
                           charactership.health, 100,
                           charactership.power, 100);

        move = readChar("Your move: ");

        if (move == 'S')
            currentspace = spaceArray[currentspace].starboard;
        else if (move == 'P')
            currentspace = spaceArray[currentspace].port;
        else if (move == 'F')
            currentspace = spaceArray[currentspace].forward;
        else if (move == 'B')
            currentspace = spaceArray[currentspace].backward;
        else if (move == 'C')
        {
            showPlayerSummary();
            continue;
        }
        else if (move == 'I')
        {
            initializeinventory();
            continue;
        }
        else if (move == 'Q')
        {
            break;
        }
        else
        {
            cout << "Please enter either 'F', 'S', 'B' or 'P'! " << endl;
            continue;
        }

        // Guard against a dead end on the map ring.
        if (currentspace < 0 || currentspace >= 17)
        {
            cout << "There is nothing that way, captain." << endl;
            currentspace = 0;
            continue;
        }

        cout << "You emerge in the " << spaceArray[currentspace].name << endl;
        cout << spaceArray[currentspace].description << endl;

        initializeBattle();
        giveItem();
        storyAdvanceChapter();
    }

    /* ---- game over ---- */
    showTargetscore();
    initializeendgame();

    cout << "Try again because there is no End to the game but for Game Over." << endl;
    return 0;
}
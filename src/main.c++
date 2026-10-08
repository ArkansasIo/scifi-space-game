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
    runLoadingScreen(9);

    loadingStep("stat weights and caps");
    loadingStep("effects, enemies and affixes");
    loadingStep("bosses, gear and sets");
    loadingStep("loot tables and encounters");

    initializeFramework();

    // Content that the procedural universe reads while generating: it needs
    // the faction table (for ownership) and the biome/class tables (for world
    // detail), so both must be populated first.
    loadingStep("biomes and classes");
    initializeBiomes();
    initializeClasses();

    loadingStep("starships and components");
    initializeStarships();
    initializeWeapons();
    initializeComponents();

    // The starting squadron reads the starship table, so it must be seeded
    // after initializeStarships() - not inside initializeFramework().
    seedPlayerFleet();

    // The 4X galaxy map (hand-authored 17 systems) and the procedural
    // universe (seeded, 32 systems with moons, gates and anomalies).
    loadingStep("4X galaxy map");
    initializeGalaxy();

    loadingStep("procedural universe");
    generateUniverse(0xB47A1EC5u);

    // Turn machine and its scheduled jobs.
    loadingStep("turn system and scheduler");
    turnInit();
    schedulerInit();
    schedulerInstallDefaults();

    // Player gameplay state: cargo, skills, standings, contracts, colonies.
    // Initialised after the universe and faction tables, which the standings
    // list is built from.
    loadingStep("player gameplay systems");
    gameplayInit();

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
        else if (move == 'M')
        {
            // Map / universe menus.
            universeMenu();
            continue;
        }
        else if (move == 'V')
        {
            // Empire / roster menus.
            empireMenu();
            continue;
        }
        else if (move == 'T')
        {
            // Turn / scheduler status.
            turnMenu();
            continue;
        }
        else if (move == 'P' || move == 'G')
        {
            // Player / gameplay systems.
            gameplayMenu();
            continue;
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
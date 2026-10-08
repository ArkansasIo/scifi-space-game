// src/game/menus.c++ -- the in-game menu screens.
//
// Every menu is a numbered list dispatched by runSubMenu(): digits pick, 0
// returns to the parent.  The screens themselves are the show*() functions
// from universe_gen.c++, biome.c++, gamelogic.c++, turn.c++ and scheduler.c++,
// so nothing here duplicates presentation logic.
//
// See docs/UI.md.

#include "spacebattlerpg.h"
#include "game/menus.h"
#include "world/universe_gen.h"
#include "ui/display.h"

/* ---------------- shared helpers ---------------- */

void menuWait()
{
    cout << endl;
    waitForEnter();
}

// Draw a numbered menu, read a choice, call the handler.  Returns on 0.
void runSubMenu(const char *title, const char *entries[],
                void (*handlers[])(), int count)
{
    for (;;)
    {
        clearScreen();

        if (title)
            drawHeader(title);

        for (int i = 0; i < count; ++i)
            cout << "   " << (i + 1) << ") " << entries[i] << endl;

        cout << "   0) Back" << endl
             << endl;

        int choice = askInt("Choice: ", 0, count);

        if (choice == 0)
            return;

        int index = choice - 1;

        if (index < 0 || index >= count)
            continue;

        if (handlers && handlers[index])
        {
            clearScreen();
            handlers[index]();
            menuWait();
        }
    }
}

/* ---------------- map sub-menus ---------------- */

void menuUniverseOverview()
{
    showUniverseOverview();
}

void menuSectorList()
{
    // List the sectors, then open the chosen one.
    drawHeader("Sectors");

    for (int i = 0; i < sectorRecordCount; ++i)
    {
        const sector &sec = sectorArray[i];

        cout << "   " << (i + 1) << ") " << sec.name
             << "  systems " << sec.firstSystem << "-"
             << (sec.firstSystem + sec.systemCount - 1)
             << "  dev " << sec.development
             << "  danger " << sec.danger << endl;
    }

    cout << "   0) Back" << endl
         << endl;

    int choice = askInt("Which sector? ", 0, sectorRecordCount);

    if (choice <= 0)
        return;

    clearScreen();
    showSector(choice - 1);
}

void menuSystemList()
{
    drawHeader("Systems");

    for (int i = 0; i < systemWorldCount; ++i)
    {
        const systemworld &sys = systemWorldArray[i];

        cout << "   " << (i + 1) << ") " << sys.name
             << "  [" << starClassName(sys.primary.klass) << "]"
             << "  worlds " << sys.worldCount
             << "  hab " << countHabitableWorlds(i)
             << "  dev " << sys.development
             << "  danger " << sys.danger << endl;
    }

    cout << "   0) Back" << endl
         << endl;

    int choice = askInt("Which system? ", 0, systemWorldCount);

    if (choice <= 0)
        return;

    clearScreen();
    showSystemWorld(choice - 1);
}

void menuCurrentSystem()
{
    // Both maps are generated: the 4X layer (systemArray, 17 hand-authored
    // systems) and the procedural universe (systemWorldArray, 32 seeded
    // systems).  Show the procedural view, since it carries the richer data
    // (star class, moons, gates, anomalies), and fall back to the 4X list if
    // the universe has not been generated.
    if (systemWorldCount > 0)
    {
        showSystemWorld(0);
        return;
    }

    showGalaxyMap();
}

void menuSystemObjects()
{
    if (systemWorldCount <= 0)
    {
        out("No systems generated.");
        return;
    }

    drawHeader("Objects");

    for (int i = 0; i < systemWorldCount; ++i)
    {
        const systemworld &sys = systemWorldArray[i];

        if (sys.anomalyCount == 0)
            continue;

        cout << "  " << sys.name << endl;
        showAnomalyList(i);
    }

    cout << endl;
}

void menuJumpRoutes()
{
    if (systemWorldCount <= 0)
    {
        out("No systems generated.");
        return;
    }

    drawHeader("Jump Routes");

    int routes[MAX_SYSTEMS_WORLDS];
    int count = reachableSystems(0, routes, MAX_SYSTEMS_WORLDS);

    cout << "  From " << systemWorldArray[0].name << ":" << endl
         << endl;

    for (int i = 0; i < count; ++i)
    {
        int target = routes[i];

        cout << "    - " << systemWorldArray[target].name
             << "  (" << systemWorldArray[target].gates[0].distance
             << " ly, security "
             << systemWorldArray[target].gates[0].security << ")" << endl;
    }

    cout << endl
         << "  Reachability from here:" << endl;

    for (int i = 0; i < systemWorldCount && i < 8; ++i)
    {
        cout << "    " << systemWorldArray[i].name
             << "  jumps " << jumpDistance(0, i) << endl;
    }

    cout << endl;
}

void universeMenu()
{
    static const char *entries[] = {
        "Universe overview",
        "Sectors",
        "Systems",
        "Current system",
        "Interstellar objects",
        "Jump routes",
        "Biome list",
        "Class tree"};

    static void (*handlers[])() = {
        menuUniverseOverview,
        menuSectorList,
        menuSystemList,
        menuCurrentSystem,
        menuSystemObjects,
        menuJumpRoutes,
        showBiomeList,
        showClassTree};

    runSubMenu("Map", entries, handlers, 8);
}

/* ---------------- empire sub-menus ---------------- */

void menuCharacter()
{
    statblock stats = buildPlayerStatBlock();

    showStatSheet(stats);
    showPlayerSummary();
}

void menuEmpireDashboard()
{
    showEmpireDashboard();
}

void menuFleet()
{
    drawHeader("Fleet");

    drawField("Ships", playerFleet.shipCount);
    drawField("Fleet power", fleetPower(playerFleet));
    drawField("Supply use", fleetSupplyUse(playerFleet));
    drawField("Command capacity", playerFleet.commandCapacity);
    drawField("Within capacity", fleetWithinCommandCapacity(playerFleet));
    drawField("Morale", playerFleet.morale);
    drawField("Sensor range", playerFleet.sensorRange);

    cout << endl;

    for (int i = 0; i < playerFleet.shipCount && i < MAX_SHIPS; ++i)
    {
        const shipinstance &ship = playerFleet.ships[i];

        cout << "    " << (i + 1) << ". " << ship.name
             << "  hull " << ship.hull
             << "  shields " << ship.shields
             << "  firepower " << ship.firepower << endl;
    }

    if (playerFleet.shipCount == 0)
        out("(no ships)");

    cout << endl;
}

void menuShipDatabase()
{
    static const char *entries[] = {
        "Hull classes",
        "Weapons",
        "Components",
        "Compare two hulls"};

    static void (*handlers[])() = {
        showShipClassList,
        showWeaponList,
        showComponentList,
        0};

    runSubMenu("Ship Database", entries, handlers, 4);
}

void menuBossRoster()
{
    showBossRoster();
}

void menuItemBrowser()
{
    drawHeader("Items");

    for (int i = 0; i < itemCount; ++i)
    {
        const itemdata &item = itemArray[i];

        cout << "   " << (i + 1) << ") " << item.name
             << "  (" << rarityName(item.rarity) << ", "
             << slotName(item.slot) << ")"
             << "  PvE " << item.itemPowerPvE
             << " / PvP " << item.itemPowerPvP << endl;
    }

    cout << "   0) Back" << endl
         << endl;

    int choice = askInt("Which item? ", 0, itemCount);

    if (choice <= 0)
        return;

    clearScreen();
    showItemTooltip(choice - 1);
}

void empireMenu()
{
    static const char *entries[] = {
        "Character sheet",
        "Empire dashboard",
        "Fleet",
        "Ship database",
        "Boss roster",
        "Item browser",
        "Encounter templates"};

    static void (*handlers[])() = {
        menuCharacter,
        menuEmpireDashboard,
        menuFleet,
        menuShipDatabase,
        menuBossRoster,
        menuItemBrowser,
        0};

    runSubMenu("Empire", entries, handlers, 7);
}

/* ---------------- turn sub-menus ---------------- */

void menuTurnStatus()
{
    showTurnStatus();
}

void menuTurnHistory()
{
    showTurnHistory();
}

void menuTurnSteps()
{
    showTurnSteps();
}

void menuScheduledJobs()
{
    showSchedulerStatus();
    showScheduledJobs();
}

void menuAdvanceTurn()
{
    drawHeader("Advance Turn");

    // Fire every scheduled job for the current turn, then advance.
    int fired = 0;

    for (int p = 0; p < TPHASE_COUNT; ++p)
        fired += schedulerRunPhase((turnphase)p, turnNumber());

    int steps = turnAdvance();

    schedulerUpdateCountdowns(turnNumber());

    drawField("Jobs fired", fired);
    drawField("Turn steps run", steps);
    drawField("Now on turn", turnNumber());
    drawFieldText("Calendar", turnSeasonName());

    cout << endl;

    if (turnHistoryCount() > 0)
    {
        out("Latest:");
        cout << "    " << turnHistory(turnHistoryCount() - 1) << endl;
    }

    cout << endl;
}

void turnMenu()
{
    static const char *entries[] = {
        "Turn status",
        "Turn history",
        "Turn steps",
        "Scheduled jobs",
        "Advance one turn"};

    static void (*handlers[])() = {
        menuTurnStatus,
        menuTurnHistory,
        menuTurnSteps,
        menuScheduledJobs,
        menuAdvanceTurn};

    runSubMenu("Turn", entries, handlers, 5);
}

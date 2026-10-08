// include/game/menus.h -- the in-game menu screens.
//
// Three top-level menus, each opening its own set of sub-menus:
//
//   M  Map        Galaxy > Sectors ... / Systems ... / Current system / Objects
//   V  Empire     Character / Empire / Fleet / Ships / Bosses / Items
//   T  Turn       Status / History / Steps / Scheduled jobs / Advance turn
//
// Every screen is a numbered list, so the wheel of keys a player has to
// remember stays small: one letter to open a menu, digits to pick, 0 to go back.
//
// See docs/UI.md.

#ifndef SBW_GAME_MENUS_H
#define SBW_GAME_MENUS_H

/* ---------------- top-level menus ---------------- */

// Map / universe: galaxy overview, sectors, systems, objects.
void universeMenu();

// Empire: character, empire, fleet, ships, bosses, items.
void empireMenu();

// Turn: status, history, steps, scheduled jobs.
void turnMenu();

/* ---------------- map sub-menus ---------------- */

void menuUniverseOverview(); // universe totals + sector list
void menuSectorList();       // pick a sector, then see its systems
void menuSystemList();       // pick a system, then see it in detail
void menuCurrentSystem();    // the system the player is in
void menuSystemObjects();    // anomalies / interstellar objects
void menuJumpRoutes();       // reachable systems and jump distances

/* ---------------- empire sub-menus ---------------- */

void menuCharacter();       // stat sheet + progression
void menuEmpireDashboard(); // attributes, resources, rank
void menuFleet();           // fleet power, supply, composition
void menuShipDatabase();    // hull classes, weapons, components
void menuBossRoster();      // every boss and its phase plan
void menuItemBrowser();     // items with PvE / PvP tooltips

/* ---------------- turn sub-menus ---------------- */

void menuTurnStatus();    // current turn, phase, calendar
void menuTurnHistory();   // the rolling log
void menuTurnSteps();     // registered per-turn steps
void menuScheduledJobs(); // the cronjob roster

/* ---------------- helpers ---------------- */

// A sub-menu loop: draws a numbered list, dispatches, returns on 0.
// `entries` is a null-terminated array of labels; `handlers` is the matching
// array of functions (null entry = no-op).
void runSubMenu(const char *title, const char *entries[],
                void (*handlers[])(), int count);

// Pause for the player, then return to the parent menu.
void menuWait();

#endif /* SBW_GAME_MENUS_H */

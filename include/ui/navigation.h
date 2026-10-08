// include/ui/navigation.h -- the screen framework: top bar, left nav bar,
// menus and sub-menus.
//
// Layout on an 80x24 terminal:
//
//   +------------------------------------------------------------------------+
//   | Space Battle Wars    Turn 12   Credits 4500   Hull 100%   [ADMIRAL]    |  <- top bar
//   +----------------+-------------------------------------------------------+
//   | > Galaxy       |                                                       |
//   |   Fleet        |                                                       |
//   |   Empire       |              content area                             |
//   |   Research     |                                                       |
//   |   Ships        |                                                       |
//   |   Accounts     |                                                       |
//   |   Options      |                                                       |
//   +----------------+-------------------------------------------------------+
//   | F1 Help  F2 Status  Esc Back                        [autosave: ON]     |  <- status bar
//   +------------------------------------------------------------------------+
//
// The nav bar is a tree: each top-level entry can own sub-menus, and a
// sub-menu can own sub-sub-menus, to any depth.  Selection is by number, so
// the whole UI is keyboard-drivable without a mouse.
//
// See docs/UI.md.

#ifndef SBW_UI_NAVIGATION_H
#define SBW_UI_NAVIGATION_H

/* ---------------- geometry ---------------- */

#define NAV_TOP_HEIGHT 1
#define NAV_STATUS_HEIGHT 1
#define NAV_SIDE_WIDTH 18
#define NAV_MAX_DEPTH 4
#define NAV_MAX_ITEMS 24

/* ---------------- screen ids ---------------- */

enum screenid
{
    SCREEN_NONE = 0,
    SCREEN_GALAXY_MAP,
    SCREEN_SYSTEM_VIEW,
    SCREEN_FLEET,
    SCREEN_SHIP_DETAIL,
    SCREEN_EMPIRE,
    SCREEN_COLONY,
    SCREEN_RESEARCH,
    SCREEN_TECH_TREE,
    SCREEN_CHARACTER,
    SCREEN_INVENTORY,
    SCREEN_EQUIPMENT,
    SCREEN_ENCOUNTER,
    SCREEN_COMBAT,
    SCREEN_STORY,
    SCREEN_ACCOUNTS,
    SCREEN_LEADERBOARD,
    SCREEN_OPTIONS,
    SCREEN_SAVE_LOAD,
    SCREEN_HELP,
    SCREEN_COUNT
};

/* ---------------- menu item ---------------- */

struct menuitem
{
    char label[32];
    char hotkey;     // 'G', 'F', ...
    screenid target; // screen this opens (SCREEN_NONE = not navigable)
    int enabled;
    int childCount; // sub-menu entries
};

/* ---------------- menu node (tree) ---------------- */

struct menunode
{
    char title[32];
    screenid screen;
    int parent;
    int depth;
    int itemCount;
    menuitem items[NAV_MAX_ITEMS];
};

/* ---------------- ui state ---------------- */

struct uistate
{
    int topBarDrawn;
    int sideBarDrawn;
    int statusBarDrawn;

    // The current navigation path, root first.
    int path[NAV_MAX_DEPTH];
    int pathDepth;
    int highlighted; // index into the current node's items
    screenid current;

    // Content scrolling.
    int scrollOffset;
    int contentLines;

    // Transient message shown in the status bar.
    char message[128];
    int messageTicks;

    int colorEnabled;
};

/* ---------------- lifecycle ---------------- */

// Build the navigation tree.
void navigationInit();

// Reset to the root.
void navigationReset();

// The root node.
const menunode *navigationRoot();

// Fetch a node by id.
const menunode *navigationNode(int nodeIndex);

// Total nodes built.
int navigationNodeCount();

/* ---------------- drawing ---------------- */

// Draw the top bar: title, turn, resources, hull, rank.
void drawTopBar(const char *title, int turn, int credits, int hullPercent,
                const char *rank);

// Draw the left nav bar for the current node, marking the selection.
void drawSideBar(const uistate &state);

// Draw the bottom status bar with the current message.
void drawStatusBar(const uistate &state);

// Draw the whole frame at once.
void drawFrame(const uistate &state, const char *title, int turn,
               int credits, int hullPercent, const char *rank);

// A horizontal menu strip for sub-menus: "1 Galaxy  2 Fleet  3 Empire".
void drawMenuStrip(const menunode &node, int selected);

// A breadcrumb line: "Empire > Colonies > Terra Prime".
void drawBreadcrumb(const uistate &state);

/* ---------------- interaction ---------------- */

// Enter a node by index.  Returns the new current screen.
screenid navigationEnter(uistate &state, int nodeIndex);

// Go back one level.  Returns 1 if it moved, 0 if already at the root.
int navigationBack(uistate &state);

// Select a numbered item in the current node.  Returns its target screen.
screenid navigationSelect(uistate &state, int itemNumber);

// Move the highlight.  Returns the new highlighted index.
int navigationMoveHighlight(uistate &state, int delta);

// Handle a single keypress.  Returns the screen to show (or SCREEN_NONE).
screenid navigationHandleKey(uistate &state, char key);

// Run the menu to a choice: draws, reads a key, returns the screen.
// Returns SCREEN_NONE on quit.
screenid navigationRunMenu(uistate &state);

/* ---------------- messages ---------------- */

void uiSetMessage(uistate &state, const char *text);
void uiTickMessage(uistate &state);
void uiClearMessage(uistate &state);

/* ---------------- content helpers ---------------- */

// Print a list of items with numbering, honouring the scroll offset.
// Returns how many lines were written.
int uiPrintList(const char *items[], int count, int selected,
                int scrollOffset, int maxLines);

// Print a two-column key/value block.
void uiPrintFields(const char *labels[], const int values[], int count);

// Right-align a value within a fixed width, writing into `out`.
void uiRightAlign(char out[], int outSize, const char *text, int width);

// Build a title bar string: "Name ......... value".
void uiTitleBar(char out[], int outSize, const char *left, const char *right);

#endif /* SBW_UI_NAVIGATION_H */

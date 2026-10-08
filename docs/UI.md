# UI and Navigation

**Modules:** `include/ui/navigation.h`, `display.h`, `hud.h`, `titlescreen.h`
**Status:** display/title/HUD implemented; navigation header implemented, source pending

---

## 1. Layout

The whole UI is designed for a fixed **80×24** terminal (`MAXCOL` × `MAXROW`
in `core/types.h`). Nothing depends on colour, cursor positioning or ANSI
escapes, so output is identical in a console, a pipe or a log file.

```
+------------------------------------------------------------------------+
| Space Battle Wars    Turn 12   Credits 4500   Hull 100%   [ADMIRAL]     |  top bar
+----------------+-------------------------------------------------------+
| > Galaxy       |                                                       |
|   Fleet        |                                                       |
|   Empire       |              content area                             |
|   Research     |                                                       |
|   Ships        |                                                       |
|   Accounts     |                                                       |
|   Options      |                                                       |
+----------------+-------------------------------------------------------+
| F1 Help  F2 Status  Esc Back                        [autosave: ON]      |  status bar
+------------------------------------------------------------------------+
```

Geometry constants:

| Constant | Value | Meaning |
| --- | --- | --- |
| `NAV_TOP_HEIGHT` | 1 | Top bar rows |
| `NAV_SIDE_WIDTH` | 18 | Left nav columns |
| `NAV_STATUS_HEIGHT` | 1 | Status bar rows |
| `NAV_MAX_DEPTH` | 4 | Max menu nesting |
| `NAV_MAX_ITEMS` | 24 | Items per menu node |

Content area: `80 − 18 = 62` columns, `24 − 3 = 21` rows.

---

## 2. Screens

21 screen ids, from `SCREEN_GALAXY_MAP` to `SCREEN_HELP`. A screen is a
*content target*; the nav tree decides which one is showing.

| Group | Screens |
| --- | --- |
| World | Galaxy Map, System View |
| Military | Fleet, Ship Detail, Encounter, Combat |
| Empire | Empire, Colony, Research, Tech Tree |
| Character | Character, Inventory, Equipment |
| Narrative | Story |
| Meta | Accounts, Leaderboard, Options, Save/Load, Help |

---

## 3. The Menu Tree

Menus are a **tree**, not a flat list. A node owns items; an item either
navigates to a child node or opens a screen.

```c
struct menuitem
{
    char label[32];
    char hotkey;        // 'G', 'F', ...
    screenid target;    // SCREEN_NONE = not navigable
    int enabled;
    int childCount;     // sub-menu entries
};

struct menunode
{
    char title[32];
    screenid screen;
    int parent;
    int depth;
    int itemCount;
    menuitem items[NAV_MAX_ITEMS];
};
```

The tree nests to `NAV_MAX_DEPTH` (4), which gives
`Empire → Colonies → Terra Prime → Overview` without any special casing.

### 3.1 Selection

Navigation is by **number**, so the whole UI is keyboard-drivable:

```
  1) Galaxy
  2) Fleet
  3) Empire
```

`navigationSelect(state, 3)` returns the target screen for item 3.

### 3.2 Back

`navigationBack(state)` pops one level and returns 1, or returns 0 when
already at the root — so the caller can treat "back at root" as "exit to
title" if it wants.

---

## 4. Drawing

| Function | Draws |
| --- | --- |
| `drawTopBar(title, turn, credits, hullPercent, rank)` | Identity, turn, resources, hull, rank |
| `drawSideBar(state)` | Left nav with the current selection marked |
| `drawStatusBar(state)` | Context keys + current message |
| `drawFrame(...)` | All three at once |
| `drawMenuStrip(node, selected)` | Horizontal sub-menu strip |
| `drawBreadcrumb(state)` | `Empire > Colonies > Terra Prime` |

A sub-menu strip:

```
1 Galaxy  2 Fleet  3 Empire  4 Research
```

### 4.1 The message line

```c
uiSetMessage(state, "Colony founded on Altair IV II.");
uiTickMessage(state);     // decrements the display duration
uiClearMessage(state);
```

Transient feedback lives in the status bar rather than scrolling the content
area. `messageTicks` expires it, so a stale message never lingers into an
unrelated screen.

---

## 5. Content Helpers

```c
int uiPrintList(const char *items[], int count, int selected,
                int scrollOffset, int maxLines);
void uiPrintFields(const char *labels[], const int values[], int count);
void uiRightAlign(char out[], int outSize, const char *text, int width);
void uiTitleBar(char out[], int outSize, const char *left, const char *right);
```

`uiPrintList` handles selection highlighting and scrolling together — the
common case of "show me 40 systems in 21 rows" is one call.

`uiTitleBar` builds the aligned header used across every panel:

```
Name ......................... value
```

---

## 6. Interaction

```c
screenid navigationHandleKey(uistate &state, char key);
screenid navigationRunMenu(uistate &state);
```

| Key | Action |
| --- | --- |
| `1`–`9` | Select item by number |
| Up/Down arrows | Move highlight |
| `Esc` | Back one level |
| `Enter` | Enter the highlighted item |
| `Q` | Quit to title |

`navigationHandleKey` returns the screen to show, or `SCREEN_NONE` if the
keypress didn't change screens — so the caller can decide whether to redraw.

---

## 7. Relationship to the Other UI Modules

| Module | Responsibility |
| --- | --- |
| `display.h` | Primitives: rules, headers, panels, bars, fields, menus, text I/O |
| `navigation.h` | The frame, the tree, and screen routing |
| `hud.h` | In-game overlay: exploration HUD, combat HUD, score strip, map panel |
| `titlescreen.h` | Bootloader, loading screen, title menu, options, credits |

`navigation.h` sits above `display.h` and uses its primitives. `hud.h` is used
by the game loop directly, since combat doesn't want a nav bar covering the
action.

---

## 8. Input Discipline

All input goes through `readInt` / `readChar` / `waitForEnter` in
`core/input.c++`, which consume whole lines. This matters more than it
sounds: a prompt that reads with `cin >> x` and leaves a newline behind will
silently desync every prompt after it.

`readChar` also strips explicit `\r`/`\n`, because CRLF-terminated redirected
input would otherwise reject valid lines. Both behaviours were bugs found
during testing, and both are now handled in one place.

---

## 9. API Summary

**Frame**

```c
void drawTopBar(const char *title, int turn, int credits, int hullPercent,
                const char *rank);
void drawSideBar(const uistate &state);
void drawStatusBar(const uistate &state);
void drawFrame(const uistate &state, const char *title, int turn,
               int credits, int hullPercent, const char *rank);
```

**Navigation**

```c
void navigationInit();
void navigationReset();
const menunode *navigationRoot();
const menunode *navigationNode(int nodeIndex);
screenid navigationEnter(uistate &state, int nodeIndex);
int navigationBack(uistate &state);
screenid navigationSelect(uistate &state, int itemNumber);
int navigationMoveHighlight(uistate &state, int delta);
screenid navigationHandleKey(uistate &state, char key);
screenid navigationRunMenu(uistate &state);
```

---

## 10. Open Questions

- **Mouse support.** Deliberately absent; the target is keyboard-only. A
  clickable region map would sit between `navigation.h` and the transport.
- **Responsive width.** `NAV_SIDE_WIDTH` is fixed at 18. A 120-column
  terminal wastes 40 columns that the content area could use.
- **Screen stack.** `path[]` is a nav path, not a history stack — pressing
  back after jumping sideways returns to the parent, not where you came from.

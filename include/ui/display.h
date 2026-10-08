// include/ui/display.h -- the presentation layer.
//
// Everything the player sees goes through here: boxed panels, tables, bars,
// colour-free terminal formatting (the engine targets a plain 80x24 console,
// see MAXROW / MAXCOL in core/types.h), plus the text input / output API.

#ifndef SBW_UI_DISPLAY_H
#define SBW_UI_DISPLAY_H

/* ---------------- canvas ---------------- */

// Clear the screen (ANSI on capable terminals, blank lines otherwise).
void clearScreen();

// Print enough blank lines to scroll the console clean.
void scrollClean();

// Draw a horizontal rule of the given character.
void drawRule(char fill);
void drawDoubleRule(char fill);

// Draw a box header: +----+ / | title | / +----+.
void drawHeader(const char *title);

// Draw a boxed panel around the given body text.
void drawPanel(const char *title, const char *body);

// Centred line, padded to the column width.
void drawCentered(const char *text);

// A labelled key/value row, aligned at the colon.
void drawField(const char *label, int value);
void drawFieldText(const char *label, const char *value);

/* ---------------- bars ---------------- */

// Draw a bar like: [##########----------]  50%
void drawBar(const char *label, int value, int maximum, int width);

// HP / SIF / power shorthand wrappers.
void drawHealthBar(int current, int maximum);
void drawShieldBar(int current, int maximum);
void drawPowerBar(int current, int maximum);

/* ---------------- text output ---------------- */

// Print a line with a consistent two-space indent.
void out(const char *text);
void outNumber(const char *label, int value);

// Prompt-and-read helpers built on core/input.h.
int askInt(const char *prompt, int minimum, int maximum);
int askYesNo(const char *prompt);
const char *askText(const char *prompt, char buffer[], int bufferSize);

// Pause until the player presses enter.
void pressEnterToContinue();

// Print a wrapped paragraph to the console width.
void outWrapped(const char *text);

/* ---------------- menus ---------------- */

// Numbered menu.  Returns the 0-based index chosen.
int showMenu(const char *title, const char *options[], int optionCount,
             int allowCancel);

// Single key choice from a set of characters.
char showKeyMenu(const char *title, const char *keys,
                 const char *descriptions[], int count);

#endif /* SBW_UI_DISPLAY_H */

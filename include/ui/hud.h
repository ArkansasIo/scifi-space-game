// include/ui/hud.h -- in-game heads-up display.
//
// The combat / exploration overlay built on top of ui/display.h and the
// framework's summary screens declared in game/gamelogic.h.

#ifndef SBW_UI_HUD_H
#define SBW_UI_HUD_H

// Draw the main exploration HUD (location, hull, order prompt).
void showExplorationHUD(const char *locationName, int hull, int maxHull,
                        int power, int maxPower);

// Draw the combat HUD for a fight against one enemy.
void showCombatHUD(const char *playerName, int playerHp, int playerMaxHp,
                   const char *enemyName, int enemyHp, int enemyMaxHp);

// Draw the score / objective strip.
void showScoreStrip(int score, int kills, int damageDone, int damageTaken);

// Draw the active modifier list (mission skulls).
void showModifierStrip();

// Draw a compact inventory panel.
void showInventoryPanel();

// Draw the map panel for the current location.
void showMapPanel(int locationIndex);

// Draw the character / ship sheet.
void showCharacterPanel();

#endif /* SBW_UI_HUD_H */

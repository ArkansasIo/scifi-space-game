// include/game/turn.h -- the turn system.
//
// A turn is the unit of world time.  Everything that happens "per turn" goes
// through here: economy ticks, research progress, fleet movement, colony
// growth, event rolls and end-of-turn saves.
//
// Phases, in order:
//
//   TURN_START -> PRODUCTION -> RESEARCH -> MOVEMENT -> EVENTS
//              -> RESOLUTION -> MAINTENANCE -> TURN_END
//
// Each phase is a registered step; steps run in priority order within a
// phase.  This is what makes the "cronjob" scheduler in game/scheduler.h
// possible: a scheduled job simply declares which phase it belongs to and how
// often it should fire (every turn, every N turns, or on a condition).
//
// See docs/TURNS.md.

#ifndef SBW_GAME_TURN_H
#define SBW_GAME_TURN_H

/* ---------------- limits ---------------- */

#define TURN_MAX_STEPS   32
#define TURN_MAX_HISTORY 64

/* ---------------- phases ---------------- */

enum turnphase
{
    TPHASE_START = 0,
    TPHASE_PRODUCTION,
    TPHASE_RESEARCH,
    TPHASE_MOVEMENT,
    TPHASE_EVENTS,
    TPHASE_RESOLUTION,
    TPHASE_MAINTENANCE,
    TPHASE_END,
    TPHASE_COUNT
};

/* ---------------- step ---------------- */

// A unit of per-turn work.
struct turnstep
{
    char name[32];
    turnphase phase;
    int priority;         // lower runs first within the phase
    int enabled;
    int runCount;
    int (*run)(int turn);
};

/* ---------------- state ---------------- */

struct turnstate
{
    int turnNumber;
    turnphase phase;
    int year;             // in-universe year
    int quarter;          // 1..4
    int paused;
    int autoEndTurn;
    int stepsRunThisTurn;
    int lastTurnDuration; // ticks

    // A short log of what happened, newest last.
    char history[TURN_MAX_HISTORY][80];
    int historyCount;
};

/* ---------------- lifecycle ---------------- */

void turnInit();
void turnReset();

// Register a step.  Returns the step index, or -1 if the table is full.
int turnRegisterStep(const char *name, turnphase phase, int priority,
                     int (*run)(int turn));

// Enable or disable a step by name.  Returns 1 on success.
int turnSetStepEnabled(const char *name, int enabled);
int turnStepEnabled(const char *name);

// Run one complete turn: every phase, every enabled step.
// Returns the number of steps that ran.
int turnAdvance();

// Run one phase only.
int turnRunPhase(turnphase phase);

// Run a single named step immediately.  Returns its result, or -1.
int turnRunStep(const char *name);

// The current state.
const turnstate &turnGetState();

/* ---------------- queries ---------------- */

int turnNumber();
int turnYear();
const char *turnPhaseName(turnphase phase);

// How many steps are registered / enabled.
int turnStepCount();
int turnEnabledStepCount();

// Fetch a step by name.  Returns null when absent.
const turnstep *turnGetStep(const char *name);

/* ---------------- history ---------------- */

void turnLog(const char *message);
int turnHistoryCount();
const char *turnHistory(int index);

/* ---------------- calendar ---------------- */

// 4 quarters per year.  Advancing past Q4 rolls the year over.
void turnAdvanceCalendar();
const char *turnSeasonName();

/* ---------------- presentation ---------------- */

void showTurnStatus();
void showTurnHistory();
void showTurnSteps();

#endif /* SBW_GAME_TURN_H */

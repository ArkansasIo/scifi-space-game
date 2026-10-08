// include/game/scheduler.h -- the scheduled-job ("cronjob") system.
//
// The turn system answers "what happens each turn".  The scheduler answers
// *when* it happens: a job declares a schedule and is fired by the clock.
//
// Schedule kinds:
//
//   EVERY_TURN        fires once per turn
//   EVERY_N_TURNS     fires every N turns (phase-locked, so it lands on a
//                     predictable turn boundary)
//   EVERY_N_TICKS     fires on a real-time tick budget
//   AT_TURN           fires on one specific turn, then disables itself
//   REPEATING_DAILY   fires once per in-universe year, in one quarter
//   ON_CONDITION      fires whenever a predicate returns true
//
// Jobs are phase-aware: a job that belongs to TPHASE_PRODUCTION runs during
// that phase, in the same order as turn steps.
//
// See docs/SCHEDULER.md.

#ifndef SBW_GAME_SCHEDULER_H
#define SBW_GAME_SCHEDULER_H

#include "game/turn.h"

/* ---------------- limits ---------------- */

#define SCHED_MAX_JOBS 48

/* ---------------- schedule kinds ---------------- */

enum schedulekind
{
    SCHED_EVERY_TURN = 0,
    SCHED_EVERY_N_TURNS,
    SCHED_EVERY_N_TICKS,
    SCHED_AT_TURN,
    SCHED_REPEATING_DAILY,
    SCHED_ON_CONDITION,
    SCHED_KIND_COUNT
};

/* ---------------- job ---------------- */

struct scheduledjob
{
    char name[32];
    schedulekind kind;
    turnphase phase;

    int interval;       // N for EVERY_N_TURNS / EVERY_N_TICKS
    int targetTurn;     // for AT_TURN
    int quarter;        // for REPEATING_DAILY (1..4)
    int ticksRemaining; // countdown for EVERY_N_TICKS
    int turnsRemaining; // countdown for EVERY_N_TURNS

    int enabled;
    int runCount;
    int failCount;
    int lastRunTurn;
    int lastRunTick;

    int (*run)(int turn);       // the job body
    int (*condition)(int turn); // predicate for ON_CONDITION
};

/* ---------------- state ---------------- */

struct schedulerstate
{
    int currentTick;
    int lastRunTurn;
    int jobsFiredThisTurn;
    int totalJobsFired;
    int totalFailures;
};

/* ---------------- lifecycle ---------------- */

void schedulerInit();
void schedulerReset();

// Schedule a job.  Returns the job index, or -1 when the table is full.
int schedulerAdd(const char *name, schedulekind kind, turnphase phase,
                 int interval, int (*run)(int turn));

// A job that fires on one specific turn.
int schedulerAddAtTurn(const char *name, turnphase phase, int targetTurn,
                       int (*run)(int turn));

// A job that fires once per in-universe year, in the given quarter.
int schedulerAddDaily(const char *name, turnphase phase, int quarter,
                      int (*run)(int turn));

// A job that fires while a predicate holds.
int schedulerAddConditional(const char *name, turnphase phase,
                            int (*condition)(int turn),
                            int (*run)(int turn));

/* ---------------- control ---------------- */

int schedulerEnable(const char *name, int enabled);
int schedulerIsEnabled(const char *name);
int schedulerRemove(const char *name);

// Fire every job due in this phase.  Returns how many ran.
int schedulerRunPhase(turnphase phase, int turn);

// Advance the real-time clock; fires tick-scheduled jobs.
int schedulerTick(int ticks, int turn);

// Reset countdowns for jobs that have just fired.
void schedulerUpdateCountdowns(int turn);

/* ---------------- queries ---------------- */

const scheduledjob *schedulerGet(const char *name);
const scheduledjob *schedulerGetIndex(int index);
const schedulerstate &schedulerGetState();

int schedulerJobCount();
int schedulerEnabledCount();
int schedulerJobsForPhase(turnphase phase);

// Is this job due right now?
int schedulerIsDue(const scheduledjob &job, int turn);

// Human-readable schedule text: "every 3 turns", "turn 40", ...
void schedulerDescribe(const scheduledjob &job, char out[], int outSize);

const char *schedulerKindName(schedulekind kind);

/* ---------------- the default roster ---------------- */

// Register the standard jobs: economy, research, colony growth, events,
// autosave, telemetry flush and the endgame countdown.
void schedulerInstallDefaults();

/* ---------------- presentation ---------------- */

void showSchedulerStatus();
void showScheduledJobs();

#endif /* SBW_GAME_SCHEDULER_H */

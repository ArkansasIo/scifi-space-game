# Scheduler (Cronjobs)

**Module:** `include/game/scheduler.h` · `src/game/scheduler.c++`
**Status:** implemented, not yet wired into the boot sequence

---

## 1. Purpose

The turn system answers *"what happens each turn"*. The scheduler answers
*"when"* — including things that don't happen every turn, or don't happen on
turn boundaries at all.

This is the job system that keeps ongoing world activity out of the turn loop:
colony growth every 3 turns, an autosave every 10, telemetry flushed every 30
real-time ticks, and war pressure that fires only while a war is actually on.

---

## 2. Schedule Kinds

Six kinds, each with its own due-check.

| Kind | Fires | Use case |
| --- | --- | --- |
| `SCHED_EVERY_TURN` | every turn | Economy, research |
| `SCHED_EVERY_N_TURNS` | every N turns, phase-locked | Colony growth (3), autosave (10) |
| `SCHED_EVERY_N_TICKS` | every N ticks of real time | Telemetry flush (30) |
| `SCHED_AT_TURN` | once, on one turn | Scripted events, endgame trigger |
| `SCHED_REPEATING_DAILY` | once per in-universe year, in one quarter | Annual reports, taxes |
| `SCHED_ON_CONDITION` | whenever a predicate returns true | War pressure, crisis events |

### 2.1 Phase-locking

`SCHED_EVERY_N_TURNS` fires when `(turn % interval) == 0` — **not** on a
countdown. A "every 3 turns" job therefore fires on turns 3, 6, 9, 12… always,
regardless of when it was registered or whether it was disabled in between.

This is the important property: a player who disables colony growth for two
turns and re-enables it doesn't shift the whole growth schedule by two turns.

### 2.2 Tick jobs and overshoot

Tick jobs count down and fire when the countdown crosses zero. The reset
carries over the overshoot:

```c
job.ticksRemaining += job.interval;
if (job.ticksRemaining <= 0)
    job.ticksRemaining = job.interval;
```

Without this, a job with a 30-tick interval could drift a tick later on every
firing and eventually skip a window entirely.

### 2.3 One-shot jobs

`SCHED_AT_TURN` disables itself after firing — it does not fire again if the
turn counter is somehow revisited.

---

## 3. Job Record

```c
struct scheduledjob
{
    char name[32];
    schedulekind kind;
    turnphase phase;        // which turn phase it belongs to

    int interval;           // N for EVERY_N_TURNS / EVERY_N_TICKS
    int targetTurn;         // for AT_TURN
    int quarter;            // for REPEATING_DAILY (1..4)
    int ticksRemaining;     // countdown
    int turnsRemaining;

    int enabled;
    int runCount;
    int failCount;          // run() returned < 0
    int lastRunTurn;
    int lastRunTick;

    int (*run)(int turn);
    int (*condition)(int turn);   // predicate for ON_CONDITION
};
```

Capacity: 48 jobs (`SCHED_MAX_JOBS`).

A job body returning a **negative** value is counted as a failure rather than
an error — the scheduler keeps running, and `failCount` shows up in the status
view so a persistently broken job is visible rather than silent.

---

## 4. Registration

```c
schedulerAdd("colony_growth", SCHED_EVERY_N_TURNS,
             TPHASE_PRODUCTION, 3, jobColonyGrowth);

schedulerAddAtTurn("act_2_opens", TPHASE_EVENTS, 40, jobActTwo);

schedulerAddDaily("annual_tax", TPHASE_MAINTENANCE, 1, jobAnnualTax);

schedulerAddConditional("war_pressure", TPHASE_EVENTS,
                        conditionAtWar, jobWarPressure);
```

---

## 5. Firing

```c
int schedulerRunPhase(turnphase phase, int turn);   // due jobs in a phase
int schedulerTick(int ticks, int turn);             // real-time clock
```

`turnAdvance()` calls `schedulerRunPhase()` once per phase, so scheduled jobs
interleave with registered turn steps in the correct phase order.

`schedulerTick()` is driven separately by the main loop, so tick-scheduled
jobs fire on wall-clock cadence rather than waiting for a turn.

---

## 6. The Default Roster

`schedulerInstallDefaults()` registers eight jobs:

| Job | Schedule | Phase | Does |
| --- | --- | --- | --- |
| `economy` | every turn | Production | `tickEmpireEconomy()` |
| `research` | every turn | Research | Accumulates tech progress |
| `colony_growth` | every 3 turns | Production | +1 development to every settled world |
| `random_event` | every 5 turns | Events | Fires a galactic event |
| `autosave` | every 10 turns | Maintenance | `initializesavegamedata()` |
| `telemetry_flush` | every 30 ticks | End | Flushes accumulated telemetry |
| `war_pressure` | on condition | Events | Fires while any faction is at war |
| `endgame` | every turn | Resolution | Triggers once kills ≥ 20 |

### 6.1 Deterministic events

`random_event` fires on `(turn % 5) == 0` **rather than rolling a random
number**. This means an otherwise identical run produces the same event
schedule, which is what makes seeded runs reproducible — and it stops the
"nothing happened for 12 turns then three events at once" clustering that
naive `rand()` produces.

### 6.2 Condition jobs

`war_pressure` demonstrates the conditional kind:

```c
static int conditionAtWar(int turn)
{
    for (int i = 0; i < factionCount; ++i)
        if (factionArray[i].atWar) return 1;
    return 0;
}
```

The job fires every turn the predicate holds. There's no built-in cooldown, so
a job that should not fire every turn must implement its own interval check
inside its body — that's a deliberate trade for simplicity.

---

## 7. Queries

```c
const scheduledjob *schedulerGet(const char *name);
const scheduledjob *schedulerGetIndex(int index);
const schedulerstate &schedulerGetState();
int schedulerJobCount();
int schedulerEnabledCount();
int schedulerJobsForPhase(turnphase phase);
int schedulerIsDue(const scheduledjob &job, int turn);
void schedulerDescribe(const scheduledjob &job, char out[], int outSize);
const char *schedulerKindName(schedulekind kind);
```

`schedulerDescribe()` produces the human text used by the status view:
`"every 3 turns (next in 2)"`, `"yearly, Q1"`, `"when condition holds"`.

---

## 8. Presentation

`showSchedulerStatus()` — job counts, fires, failures, tick.
`showScheduledJobs()` — one line per job:

```
  [on] colony_growth   every 3 turns (next in 2)  (Production)  runs 14
```

---

## 9. API Summary

```c
void schedulerInit();
void schedulerReset();
int  schedulerAdd(const char *name, schedulekind kind, turnphase phase,
                  int interval, int (*run)(int turn));
int  schedulerAddAtTurn(const char *name, turnphase phase, int targetTurn,
                        int (*run)(int turn));
int  schedulerAddDaily(const char *name, turnphase phase, int quarter,
                       int (*run)(int turn));
int  schedulerAddConditional(const char *name, turnphase phase,
                             int (*condition)(int turn),
                             int (*run)(int turn));
int  schedulerEnable(const char *name, int enabled);
int  schedulerRemove(const char *name);
int  schedulerRunPhase(turnphase phase, int turn);
int  schedulerTick(int ticks, int turn);
void schedulerInstallDefaults();
```

---

## 10. Open Questions

- **Per-job cooldowns.** Condition jobs have no built-in rate limit. A
  `cooldownTurns` field would let a job fire at most once per N turns while
  its predicate holds.
- **Job dependencies.** Nothing can express "run history flush *after*
  telemetry write". Chaining would need an `after` field or an ordering pass.
- **Persistence.** Jobs reset on load. Their countdowns and run counts would
  need to be saved for a long-running world to survive a restart.

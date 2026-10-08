# Turn System

**Module:** `include/game/turn.h` · `src/game/turn.c++`
**Status:** implemented, not yet wired into the boot sequence

---

## 1. Purpose

A turn is the unit of world time. Everything that happens "per turn" goes
through here: economy ticks, research progress, fleet movement, colony growth,
event rolls, end-of-turn saves.

The key design decision: **a turn is a sequence of phases, and each phase runs
an ordered list of registered steps.** That makes adding per-turn behaviour a
registration call rather than an edit to a giant `advanceTurn()` function.

---

## 2. Phases

Eight phases, always in this order:

| # | Phase | What belongs here |
| --- | --- | --- |
| 0 | `TPHASE_START` | Bookkeeping, reset per-turn counters |
| 1 | `TPHASE_PRODUCTION` | Economy, mining, colony output |
| 2 | `TPHASE_RESEARCH` | Tech progress, breakthrough rolls |
| 3 | `TPHASE_MOVEMENT` | Fleet travel, gate transit |
| 4 | `TPHASE_EVENTS` | Random events, war pressure, narrative |
| 5 | `TPHASE_RESOLUTION` | Combat outcomes, contested systems |
| 6 | `TPHASE_MAINTENANCE` | Upkeep, repairs, autosave |
| 7 | `TPHASE_END` | Telemetry, end-of-turn flush |

The order is deliberate: **produce before you spend, resolve before you
maintain.** Upkeep is charged against income already generated this turn, so a
player is never billed for resources they haven't received yet.

---

## 3. Steps

```c
struct turnstep
{
    char name[32];
    turnphase phase;
    int priority;     // lower runs first within the phase
    int enabled;
    int runCount;
    int (*run)(int turn);
};
```

### 3.1 Registration

```c
turnRegisterStep("economy", TPHASE_PRODUCTION, 10, jobEconomy);
```

Steps are **insertion-sorted on registration** so execution order is
deterministic regardless of registration order. Within a phase, lower priority
runs first; equal priorities keep registration order.

This matters because a player can disable steps (see §6) and re-enable them
in any order — the sequence must not depend on how they got there.

### 3.2 Capacity

32 steps maximum (`TURN_MAX_STEPS`).

---

## 4. Running a Turn

```c
int turnAdvance()
{
    if (g_turn.paused) return 0;

    for (int p = 0; p < TPHASE_COUNT; ++p)
    {
        g_turn.phase = (turnphase)p;
        turnRunPhase((turnphase)p);
    }

    g_turn.turnNumber++;
    turnAdvanceCalendar();
    g_turn.phase = TPHASE_START;
    return ran;
}
```

Returns how many steps actually ran — useful for tests asserting that a turn
did real work rather than silently no-oping.

Individual phases and steps are callable in isolation:

```c
turnRunPhase(TPHASE_PRODUCTION);     // just production
turnRunStep("economy");              // just one step
```

This is what lets the debug UI run a single step to observe its effect.

---

## 5. Calendar

Four quarters per year. Advancing past Q4 rolls the year:

```
Year 2380 Q4  →  Year 2381 Q1
```

Quarter names are seasonal (`Q1 (Spring)` … `Q4 (Winter)`), which the
scheduler uses for yearly jobs.

Starting date: **2380 Q1**, chosen to fit the story's timeline.

---

## 6. Step Control

```c
turnSetStepEnabled("random_event", 0);   // switch off events
turnStepEnabled("random_event");         // → 0
```

Disabling is what makes the game playable in different modes — a player who
wants pure strategy turns off `random_event`; a speedrun turns off `autosave`.

---

## 7. History

The turn keeps a rolling log of 64 entries (`TURN_MAX_HISTORY`). When full,
the buffer shifts left so the newest entry always fits.

```c
turnLog("Economy produced resources.");
```

Anything worth telling the player about goes here, so there's one place to
look when something unexpected happens — rather than scattering `cout` calls
through the step bodies.

---

## 8. Relationship to the Scheduler

The turn system answers *"what happens each turn"*. The scheduler
(`game/scheduler.h`) answers *"when"*.

Every scheduled job declares a `turnphase`, and fires during that phase. So:

```
turnAdvance()
  └─ TPHASE_PRODUCTION
       ├─ step "economy"            (registered directly)
       └─ job "colony_growth"       (scheduled, every 3 turns)
```

Steps and jobs share a phase and run in the same pass. The difference is only
that a step runs every turn and a job runs on a schedule.

---

## 9. Presentation

| Function | Output |
| --- | --- |
| `showTurnStatus()` | Turn, year, quarter, current phase, steps run |
| `showTurnHistory()` | The rolling log |
| `showTurnSteps()` | Every registered step with phase, priority, run count |

`turnPhaseName()` gives the display string for a phase.

---

## 10. API Summary

```c
void turnInit();
void turnReset();
int  turnRegisterStep(const char *name, turnphase phase, int priority,
                      int (*run)(int turn));
int  turnSetStepEnabled(const char *name, int enabled);
int  turnStepEnabled(const char *name);
int  turnAdvance();
int  turnRunPhase(turnphase phase);
int  turnRunStep(const char *name);
const turnstate &turnGetState();
int  turnNumber();
int  turnYear();
const char *turnPhaseName(turnphase phase);
int  turnStepCount();
int  turnEnabledStepCount();
const turnstep *turnGetStep(const char *name);
void turnLog(const char *message);
int  turnHistoryCount();
const char *turnHistory(int index);
void turnAdvanceCalendar();
const char *turnSeasonName();
```

---

## 11. Open Questions

- **Simultaneous turns.** The model is strictly sequential. Real 4X games
  often want simultaneous resolution with a conflict step — that would mean
  reordering `TPHASE_RESOLUTION` to consider all actors at once rather than
  one at a time.
- **Deeper calendars.** Only quarters exist. Weeks or days would let jobs
  schedule finer than a turn.
- **Step rollback.** A step that half-succeeds (some colonies grow, then one
  errors) leaves inconsistent state. Per-step transactions would need a
  snapshot mechanism the turn struct doesn't have.

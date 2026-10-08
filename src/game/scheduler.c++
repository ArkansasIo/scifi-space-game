// src/game/scheduler.c++ -- the scheduled-job ("cronjob") system.
//
// Jobs declare a schedule and are fired by the turn clock and the tick clock.
// See docs/SCHEDULER.md.

#include "spacebattlerpg.h"
#include "game/scheduler.h"
#include "world/universe_gen.h"

/* ---------------- storage ---------------- */

static scheduledjob g_jobs[SCHED_MAX_JOBS];
static int g_jobCount = 0;
static schedulerstate g_state;

/* ---------------- helpers ---------------- */

static void copyString(char *dest, int destSize, const char *src)
{
    if (!dest || destSize <= 0)
        return;

    strncpy(dest, src ? src : "", destSize - 1);
    dest[destSize - 1] = '\0';
}

static scheduledjob *findJob(const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < g_jobCount; ++i)
    {
        if (strcmp(g_jobs[i].name, name) == 0)
            return &g_jobs[i];
    }

    return 0;
}

/* ---------------- lifecycle ---------------- */

void schedulerInit()
{
    memset(g_jobs, 0, sizeof(g_jobs));
    memset(&g_state, 0, sizeof(g_state));

    g_jobCount = 0;
    g_state.lastRunTurn = 0;
}

void schedulerReset()
{
    schedulerInit();
}

/* ---------------- scheduling ---------------- */

// Shared job-slot allocation and field setup.
static int schedulerAllocate(const char *name, schedulekind kind,
                             turnphase phase, int (*run)(int turn))
{
    if (g_jobCount >= SCHED_MAX_JOBS)
        return -1;

    if (!name || name[0] == '\0')
        return -1;

    scheduledjob &job = g_jobs[g_jobCount];

    memset(&job, 0, sizeof(job));
    copyString(job.name, sizeof(job.name), name);

    job.kind = kind;
    job.phase = phase;
    job.enabled = 1;
    job.lastRunTurn = -1;
    job.lastRunTick = -1;
    job.run = run;

    int index = g_jobCount;
    g_jobCount++;

    return index;
}

int schedulerAdd(const char *name, schedulekind kind, turnphase phase,
                 int interval, int (*run)(int turn))
{
    int index = schedulerAllocate(name, kind, phase, run);

    if (index < 0)
        return -1;

    scheduledjob &job = g_jobs[index];

    job.interval = (interval > 0) ? interval : 1;

    // Countdowns start at full interval so the first firing is one interval
    // away, not immediately.
    job.turnsRemaining = job.interval;
    job.ticksRemaining = job.interval;

    return index;
}

int schedulerAddAtTurn(const char *name, turnphase phase, int targetTurn,
                       int (*run)(int turn))
{
    int index = schedulerAllocate(name, SCHED_AT_TURN, phase, run);

    if (index < 0)
        return -1;

    g_jobs[index].targetTurn = targetTurn;
    return index;
}

int schedulerAddDaily(const char *name, turnphase phase, int quarter,
                      int (*run)(int turn))
{
    int index = schedulerAllocate(name, SCHED_REPEATING_DAILY, phase, run);

    if (index < 0)
        return -1;

    int q = quarter;

    if (q < 1)
        q = 1;
    if (q > 4)
        q = 4;

    g_jobs[index].quarter = q;
    return index;
}

int schedulerAddConditional(const char *name, turnphase phase,
                            int (*condition)(int turn),
                            int (*run)(int turn))
{
    int index = schedulerAllocate(name, SCHED_ON_CONDITION, phase, run);

    if (index < 0)
        return -1;

    g_jobs[index].condition = condition;
    return index;
}

/* ---------------- control ---------------- */

int schedulerEnable(const char *name, int enabled)
{
    scheduledjob *job = findJob(name);

    if (!job)
        return 0;

    job->enabled = enabled ? 1 : 0;
    return 1;
}

int schedulerIsEnabled(const char *name)
{
    scheduledjob *job = findJob(name);

    if (!job)
        return 0;

    return job->enabled;
}

int schedulerRemove(const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < g_jobCount; ++i)
    {
        if (strcmp(g_jobs[i].name, name) != 0)
            continue;

        // Shift the tail down.
        for (int j = i; j < g_jobCount - 1; ++j)
            g_jobs[j] = g_jobs[j + 1];

        g_jobCount--;
        return 1;
    }

    return 0;
}

/* ---------------- due checks ---------------- */

int schedulerIsDue(const scheduledjob &job, int turn)
{
    if (!job.enabled)
        return 0;

    switch (job.kind)
    {
    case SCHED_EVERY_TURN:
        return 1;

    case SCHED_EVERY_N_TURNS:
        // Phase-locked: fires when the turn counter lands on the interval,
        // so a "every 3 turns" job always fires on turns 3, 6, 9 ...
        if (job.interval <= 1)
            return 1;

        return (turn % job.interval) == 0;

    case SCHED_EVERY_N_TICKS:
        return (job.ticksRemaining <= 0) ? 1 : 0;

    case SCHED_AT_TURN:
        return (turn == job.targetTurn) ? 1 : 0;

    case SCHED_REPEATING_DAILY:
        // Once per in-universe year, in the nominated quarter.
        return (turnGetState().quarter == job.quarter) ? 1 : 0;

    case SCHED_ON_CONDITION:
        if (job.condition)
            return job.condition(turn) ? 1 : 0;

        return 0;

    default:
        return 0;
    }
}

/* ---------------- firing ---------------- */

int schedulerRunPhase(turnphase phase, int turn)
{
    int fired = 0;

    for (int i = 0; i < g_jobCount; ++i)
    {
        scheduledjob &job = g_jobs[i];

        if (!job.enabled || job.phase != phase)
            continue;

        if (!schedulerIsDue(job, turn))
            continue;

        if (job.run)
        {
            int result = job.run(turn);

            if (result < 0)
                job.failCount++;

            job.runCount++;
        }

        job.lastRunTurn = turn;
        job.lastRunTick = g_state.currentTick;

        // A one-shot disables itself after firing.
        if (job.kind == SCHED_AT_TURN)
            job.enabled = 0;

        fired++;
        g_state.jobsFiredThisTurn++;
        g_state.totalJobsFired++;
    }

    return fired;
}

void schedulerUpdateCountdowns(int turn)
{
    for (int i = 0; i < g_jobCount; ++i)
    {
        scheduledjob &job = g_jobs[i];

        if (!job.enabled)
            continue;

        if (job.kind == SCHED_EVERY_N_TURNS && job.interval > 1)
        {
            if ((turn % job.interval) == 0)
                job.turnsRemaining = job.interval;
            else
                job.turnsRemaining--;
        }

        if (job.turnsRemaining < 0)
            job.turnsRemaining = job.interval;
    }
}

int schedulerTick(int ticks, int turn)
{
    if (ticks <= 0)
        ticks = 1;

    g_state.currentTick += ticks;

    int fired = 0;

    for (int i = 0; i < g_jobCount; ++i)
    {
        scheduledjob &job = g_jobs[i];

        if (!job.enabled || job.kind != SCHED_EVERY_N_TICKS)
            continue;

        job.ticksRemaining -= ticks;

        if (job.ticksRemaining <= 0)
        {
            if (job.run)
            {
                if (job.run(turn) < 0)
                    job.failCount++;

                job.runCount++;
            }

            job.lastRunTick = g_state.currentTick;

            // Reset the countdown, carrying over any overshoot so a job never
            // drifts behind the clock.
            job.ticksRemaining += job.interval;

            if (job.ticksRemaining <= 0)
                job.ticksRemaining = job.interval;

            fired++;
            g_state.totalJobsFired++;
        }
    }

    schedulerUpdateCountdowns(turn);
    g_state.lastRunTurn = turn;

    return fired;
}

/* ---------------- queries ---------------- */

const scheduledjob *schedulerGet(const char *name)
{
    return findJob(name);
}

const scheduledjob *schedulerGetIndex(int index)
{
    if (index < 0 || index >= g_jobCount)
        return 0;

    return &g_jobs[index];
}

const schedulerstate &schedulerGetState()
{
    return g_state;
}

int schedulerJobCount()
{
    return g_jobCount;
}

int schedulerEnabledCount()
{
    int enabled = 0;

    for (int i = 0; i < g_jobCount; ++i)
    {
        if (g_jobs[i].enabled)
            enabled++;
    }

    return enabled;
}

int schedulerJobsForPhase(turnphase phase)
{
    int count = 0;

    for (int i = 0; i < g_jobCount; ++i)
    {
        if (g_jobs[i].phase == phase)
            count++;
    }

    return count;
}

const char *schedulerKindName(schedulekind kind)
{
    switch (kind)
    {
    case SCHED_EVERY_TURN:
        return "every turn";
    case SCHED_EVERY_N_TURNS:
        return "every N turns";
    case SCHED_EVERY_N_TICKS:
        return "every N ticks";
    case SCHED_AT_TURN:
        return "at turn";
    case SCHED_REPEATING_DAILY:
        return "yearly";
    case SCHED_ON_CONDITION:
        return "on condition";
    default:
        return "unknown";
    }
}

void schedulerDescribe(const scheduledjob &job, char out[], int outSize)
{
    if (!out || outSize <= 0)
        return;

    switch (job.kind)
    {
    case SCHED_EVERY_TURN:
        snprintf(out, outSize, "every turn");
        break;

    case SCHED_EVERY_N_TURNS:
        snprintf(out, outSize, "every %d turns (next in %d)",
                 job.interval, job.turnsRemaining);
        break;

    case SCHED_EVERY_N_TICKS:
        snprintf(out, outSize, "every %d ticks (next in %d)",
                 job.interval, job.ticksRemaining);
        break;

    case SCHED_AT_TURN:
        snprintf(out, outSize, "turn %d", job.targetTurn);
        break;

    case SCHED_REPEATING_DAILY:
        snprintf(out, outSize, "yearly, Q%d", job.quarter);
        break;

    case SCHED_ON_CONDITION:
        snprintf(out, outSize, "when condition holds");
        break;

    default:
        snprintf(out, outSize, "?");
        break;
    }
}

/* ---------------- the default roster ---------------- */

// ---- job bodies ----

static int jobEconomy(int turn)
{
    (void)turn;
    tickEmpireEconomy();
    turnLog("Economy produced resources.");
    return 1;
}

static int jobResearch(int turn)
{
    static int progress = 0;

    progress += 10 + (turn / 10);

    if (progress >= 100)
    {
        progress = 0;
        turnLog("Research completed a technology.");
        return 1;
    }

    return 0;
}

static int jobColonyGrowth(int turn)
{
    (void)turn;

    // Every settled world grows a little.
    for (int i = 0; i < systemWorldCount; ++i)
    {
        systemworld &sys = systemWorldArray[i];

        if (sys.development < 100)
            sys.development++;
    }

    return 1;
}

static int jobRandomEvent(int turn)
{
    // A one-in-five chance of an event, rolled from the turn number so it is
    // reproducible from a seed rather than wall-clock random.
    if ((turn % 5) != 0)
        return 0;

    turnLog("A galactic event occurs.");
    luaFireEvent(SBW_EVENT_PVP_FLAG, turn, 0);

    return 1;
}

static int jobAutosave(int turn)
{
    initializesavegamedata();
    (void)turn;

    turnLog("Autosaved.");
    return 1;
}

static int jobTelemetryFlush(int turn)
{
    static int pending = 0;

    pending += targetScore.actions_taken;

    if (pending < 50)
        return 0;

    pending = 0;
    (void)turn;

    return 1;
}

static int jobEndgameCountdown(int turn)
{
    // The endgame begins once the fleet has proven itself.
    if (targetScore.kills >= 20)
    {
        turnLog("The Divine Order has taken notice of you.");
        return 1;
    }

    (void)turn;
    return 0;
}

// ---- condition predicates ----

static int conditionAtWar(int turn)
{
    (void)turn;

    for (int i = 0; i < factionCount; ++i)
    {
        if (factionArray[i].atWar)
            return 1;
    }

    return 0;
}

static int jobWarPressure(int turn)
{
    turnLog("Hostile fleets press the border.");
    (void)turn;
    return 1;
}

void schedulerInstallDefaults()
{
    // Economy: every turn, during production.
    schedulerAdd("economy", SCHED_EVERY_TURN, TPHASE_PRODUCTION, 1,
                 jobEconomy);

    // Research: every turn, during the research phase.
    schedulerAdd("research", SCHED_EVERY_TURN, TPHASE_RESEARCH, 1,
                 jobResearch);

    // Colony growth: every 3 turns, phase-locked.
    schedulerAdd("colony_growth", SCHED_EVERY_N_TURNS, TPHASE_PRODUCTION, 3,
                 jobColonyGrowth);

    // A random event roughly every 5 turns.
    schedulerAdd("random_event", SCHED_EVERY_N_TURNS, TPHASE_EVENTS, 5,
                 jobRandomEvent);

    // Autosave: every 10 turns, during maintenance.
    schedulerAdd("autosave", SCHED_EVERY_N_TURNS, TPHASE_MAINTENANCE, 10,
                 jobAutosave);

    // Telemetry flush: every 30 ticks of real time.
    schedulerAdd("telemetry_flush", SCHED_EVERY_N_TICKS, TPHASE_END, 30,
                 jobTelemetryFlush);

    // War pressure: fires while any faction is at war with us.
    schedulerAddConditional("war_pressure", TPHASE_EVENTS,
                            conditionAtWar, jobWarPressure);

    // Endgame trigger: fires when the kill count reaches the threshold.
    schedulerAdd("endgame", SCHED_EVERY_TURN, TPHASE_RESOLUTION, 1,
                 jobEndgameCountdown);
}

/* ---------------- presentation ---------------- */

void showSchedulerStatus()
{
    drawHeader("Scheduler");

    drawField("Jobs", schedulerJobCount());
    drawField("Enabled", schedulerEnabledCount());
    drawField("Fired this turn", g_state.jobsFiredThisTurn);
    drawField("Fired total", g_state.totalJobsFired);
    drawField("Failures", g_state.totalFailures);
    drawField("Tick", g_state.currentTick);

    cout << endl;
}

void showScheduledJobs()
{
    drawHeader("Scheduled Jobs");

    for (int i = 0; i < g_jobCount; ++i)
    {
        const scheduledjob &job = g_jobs[i];

        cout << "  ";
        if (job.enabled)
            cout << "[on] ";
        else
            cout << "[--] ";

        cout << job.name;

        int length = (int)strlen(job.name);
        for (int pad = length; pad < 18; ++pad)
            cout << ' ';

        char schedule[64];
        schedulerDescribe(job, schedule, sizeof(schedule));

        cout << schedule
             << "  (" << turnPhaseName(job.phase) << ")"
             << "  runs " << job.runCount;

        if (job.failCount > 0)
            cout << "  fails " << job.failCount;

        cout << endl;
    }

    cout << endl;
}

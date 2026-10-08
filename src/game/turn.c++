// src/game/turn.c++ -- the turn system.
//
// Owns the turn counter, the phase machine and the registered per-turn steps.
// Everything that happens "per turn" is a step; see docs/TURNS.md.

#include "spacebattlerpg.h"
#include "game/turn.h"

/* ---------------- storage ---------------- */

static turnstep g_steps[TURN_MAX_STEPS];
static int g_stepCount = 0;
static turnstate g_turn;

/* ---------------- helpers ---------------- */

static void copyString(char *dest, int destSize, const char *src)
{
    if (!dest || destSize <= 0)
        return;

    strncpy(dest, src ? src : "", destSize - 1);
    dest[destSize - 1] = '\0';
}

static turnstep *findStep(const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < g_stepCount; ++i)
    {
        if (strcmp(g_steps[i].name, name) == 0)
            return &g_steps[i];
    }

    return 0;
}

/* ---------------- lifecycle ---------------- */

void turnInit()
{
    memset(g_steps, 0, sizeof(g_steps));
    g_stepCount = 0;

    memset(&g_turn, 0, sizeof(g_turn));

    g_turn.turnNumber = 1;
    g_turn.phase = TPHASE_START;
    g_turn.year = 2380;
    g_turn.quarter = 1;
    g_turn.paused = 0;
}

void turnReset()
{
    turnInit();
}

/* ---------------- registration ---------------- */

int turnRegisterStep(const char *name, turnphase phase, int priority,
                     int (*run)(int turn))
{
    if (g_stepCount >= TURN_MAX_STEPS)
        return -1;

    if (!name || name[0] == '\0')
        return -1;

    turnstep &step = g_steps[g_stepCount];

    memset(&step, 0, sizeof(step));
    copyString(step.name, sizeof(step.name), name);

    step.phase = phase;
    step.priority = priority;
    step.enabled = 1;
    step.runCount = 0;
    step.run = run;

    // Keep each phase ordered by priority so execution order is deterministic
    // and independent of registration order.
    int index = g_stepCount;
    g_stepCount++;

    for (int i = index; i > 0; --i)
    {
        if (g_steps[i - 1].phase < g_steps[i].phase)
            break;

        if (g_steps[i - 1].phase == g_steps[i].phase
            && g_steps[i - 1].priority <= g_steps[i].priority)
            break;

        turnstep swap = g_steps[i - 1];
        g_steps[i - 1] = g_steps[i];
        g_steps[i] = swap;
    }

    return index;
}

int turnSetStepEnabled(const char *name, int enabled)
{
    turnstep *step = findStep(name);

    if (!step)
        return 0;

    step->enabled = enabled ? 1 : 0;
    return 1;
}

int turnStepEnabled(const char *name)
{
    turnstep *step = findStep(name);

    if (!step)
        return 0;

    return step->enabled;
}

/* ---------------- running ---------------- */

int turnRunPhase(turnphase phase)
{
    int ran = 0;

    for (int i = 0; i < g_stepCount; ++i)
    {
        turnstep &step = g_steps[i];

        if (step.phase != phase || !step.enabled)
            continue;

        if (step.run)
        {
            step.run(g_turn.turnNumber);
            step.runCount++;
        }

        ran++;
        g_turn.stepsRunThisTurn++;
    }

    return ran;
}

int turnRunStep(const char *name)
{
    turnstep *step = findStep(name);

    if (!step)
        return -1;

    if (!step->run)
        return -1;

    step->runCount++;
    return step->run(g_turn.turnNumber);
}

int turnAdvance()
{
    if (g_turn.paused)
        return 0;

    g_turn.stepsRunThisTurn = 0;

    // Run the phases in order.
    for (int p = 0; p < TPHASE_COUNT; ++p)
    {
        g_turn.phase = (turnphase)p;
        turnRunPhase((turnphase)p);
    }

    int ran = g_turn.stepsRunThisTurn;

    // The turn itself advances, then the calendar.
    g_turn.turnNumber++;
    turnAdvanceCalendar();

    g_turn.phase = TPHASE_START;

    return ran;
}

/* ---------------- queries ---------------- */

const turnstate &turnGetState()
{
    return g_turn;
}

int turnNumber()
{
    return g_turn.turnNumber;
}

int turnYear()
{
    return g_turn.year;
}

const char *turnPhaseName(turnphase phase)
{
    switch (phase)
    {
    case TPHASE_START:
        return "Start";
    case TPHASE_PRODUCTION:
        return "Production";
    case TPHASE_RESEARCH:
        return "Research";
    case TPHASE_MOVEMENT:
        return "Movement";
    case TPHASE_EVENTS:
        return "Events";
    case TPHASE_RESOLUTION:
        return "Resolution";
    case TPHASE_MAINTENANCE:
        return "Maintenance";
    case TPHASE_END:
        return "End";
    default:
        return "Unknown";
    }
}

int turnStepCount()
{
    return g_stepCount;
}

int turnEnabledStepCount()
{
    int enabled = 0;

    for (int i = 0; i < g_stepCount; ++i)
    {
        if (g_steps[i].enabled)
            enabled++;
    }

    return enabled;
}

const turnstep *turnGetStep(const char *name)
{
    return findStep(name);
}

/* ---------------- history ---------------- */

void turnLog(const char *message)
{
    if (!message)
        return;

    if (g_turn.historyCount >= TURN_MAX_HISTORY)
    {
        // Shift the buffer so the newest entry always fits.
        for (int i = 1; i < TURN_MAX_HISTORY; ++i)
            memcpy(g_turn.history[i - 1], g_turn.history[i],
                   sizeof(g_turn.history[0]));

        g_turn.historyCount = TURN_MAX_HISTORY - 1;
    }

    copyString(g_turn.history[g_turn.historyCount],
               sizeof(g_turn.history[0]), message);

    g_turn.historyCount++;
}

int turnHistoryCount()
{
    return g_turn.historyCount;
}

const char *turnHistory(int index)
{
    if (index < 0 || index >= g_turn.historyCount)
        return "";

    return g_turn.history[index];
}

/* ---------------- calendar ---------------- */

void turnAdvanceCalendar()
{
    g_turn.quarter++;

    if (g_turn.quarter > 4)
    {
        g_turn.quarter = 1;
        g_turn.year++;
    }
}

const char *turnSeasonName()
{
    switch (g_turn.quarter)
    {
    case 1:
        return "Q1 (Spring)";
    case 2:
        return "Q2 (Summer)";
    case 3:
        return "Q3 (Autumn)";
    case 4:
        return "Q4 (Winter)";
    default:
        return "Q?";
    }
}

/* ---------------- presentation ---------------- */

void showTurnStatus()
{
    drawHeader("Turn");

    drawField("Turn", g_turn.turnNumber);
    drawField("Year", g_turn.year);
    drawFieldText("Quarter", turnSeasonName());
    drawFieldText("Phase", turnPhaseName(g_turn.phase));
    drawField("Steps run", g_turn.stepsRunThisTurn);

    cout << endl;
}

void showTurnHistory()
{
    drawHeader("Turn History");

    for (int i = 0; i < g_turn.historyCount; ++i)
        cout << "  " << g_turn.history[i] << endl;

    if (g_turn.historyCount == 0)
        out("(nothing has happened yet)");

    cout << endl;
}

void showTurnSteps()
{
    drawHeader("Turn Steps");

    for (int i = 0; i < g_stepCount; ++i)
    {
        const turnstep &step = g_steps[i];

        cout << "  ";
        if (step.enabled)
            cout << "[on] ";
        else
            cout << "[--] ";

        cout << step.name;

        int length = (int)strlen(step.name);
        for (int pad = length; pad < 22; ++pad)
            cout << ' ';

        cout << turnPhaseName(step.phase)
             << "  pri " << step.priority
             << "  runs " << step.runCount << endl;
    }

    cout << endl;
}

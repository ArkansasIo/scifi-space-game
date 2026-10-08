// src/engine/subsystem.c++ -- the engine kernel.
//
// Owns the subsystem registry and drives the four-phase lifecycle in a fixed
// order.  Subsystems register with a priority; kernelInitAll() and
// kernelStartAll() walk them in priority order, kernelShutdownAll() walks them
// in reverse so dependencies come down before the things they depend on.

#include "spacebattlerpg.h"
#include "engine/subsystem.h"

/* ---------------- registry ---------------- */

static subsystem g_subsystems[SYS_COUNT];
static int g_subsystemCount = 0;

/* ---------------- registration ---------------- */

int kernelRegister(const subsystem &sys)
{
    if (g_subsystemCount >= SYS_COUNT)
        return 0;

    g_subsystems[g_subsystemCount] = sys;
    g_subsystems[g_subsystemCount].initialized = 0;
    g_subsystems[g_subsystemCount].running = 0;

    g_subsystemCount++;
    return 1;
}

// Insertion sort by priority, ascending.  Stable, and the registry is tiny.
static void sortByPriority()
{
    for (int i = 1; i < g_subsystemCount; ++i)
    {
        subsystem key = g_subsystems[i];
        int j = i - 1;

        while (j >= 0 && g_subsystems[j].priority > key.priority)
        {
            g_subsystems[j + 1] = g_subsystems[j];
            j--;
        }

        g_subsystems[j + 1] = key;
    }
}

/* ---------------- lifecycle ---------------- */

int kernelInitAll()
{
    sortByPriority();

    int ok = 0;

    for (int i = 0; i < g_subsystemCount; ++i)
    {
        subsystem &sys = g_subsystems[i];

        if (sys.init)
        {
            sys.initialized = sys.init();
        }
        else
        {
            sys.initialized = 1;
        }

        if (sys.initialized)
            ok++;
    }

    return ok;
}

int kernelStartAll()
{
    int ok = 0;

    for (int i = 0; i < g_subsystemCount; ++i)
    {
        subsystem &sys = g_subsystems[i];

        if (!sys.initialized)
            continue;

        if (sys.start)
        {
            sys.running = sys.start();
        }
        else
        {
            sys.running = 1;
        }

        if (sys.running)
            ok++;
    }

    return ok;
}

int kernelUpdateAll(int ticks)
{
    if (ticks <= 0)
        ticks = 1;

    int updated = 0;

    for (int i = 0; i < g_subsystemCount; ++i)
    {
        subsystem &sys = g_subsystems[i];

        if (!sys.running)
            continue;

        if (sys.update)
            sys.update(ticks);

        updated++;
    }

    return updated;
}

void kernelShutdownAll()
{
    // Reverse order: dependents come down before their dependencies.
    for (int i = g_subsystemCount - 1; i >= 0; --i)
    {
        subsystem &sys = g_subsystems[i];

        if (sys.running && sys.stop)
            sys.stop();

        sys.running = 0;

        if (sys.initialized && sys.shutdown)
            sys.shutdown();

        sys.initialized = 0;
    }

    g_subsystemCount = 0;
}

/* ---------------- lookup ---------------- */

const subsystem *kernelGet(subsystemid id)
{
    for (int i = 0; i < g_subsystemCount; ++i)
    {
        if (g_subsystems[i].id == id)
            return &g_subsystems[i];
    }

    return 0;
}

const subsystem *kernelGetByName(const char *name)
{
    if (!name)
        return 0;

    for (int i = 0; i < g_subsystemCount; ++i)
    {
        if (g_subsystems[i].name && strcmp(g_subsystems[i].name, name) == 0)
            return &g_subsystems[i];
    }

    return 0;
}

/* ---------------- diagnostics ---------------- */

int kernelSubsystemCount()
{
    return g_subsystemCount;
}

int kernelRunningCount()
{
    int running = 0;

    for (int i = 0; i < g_subsystemCount; ++i)
    {
        if (g_subsystems[i].running)
            running++;
    }

    return running;
}

void kernelDumpStatus()
{
    cout << "  Engine subsystems: " << kernelRunningCount() << "/"
         << g_subsystemCount << " running" << endl;
    cout << endl;

    for (int i = 0; i < g_subsystemCount; ++i)
    {
        const subsystem &sys = g_subsystems[i];

        cout << "    ";
        if (sys.running)
            cout << "[run] ";
        else if (sys.initialized)
            cout << "[init]";
        else
            cout << "[stop]";

        cout << " " << (sys.name ? sys.name : "(unnamed)") << endl;
    }

    cout << endl;
}

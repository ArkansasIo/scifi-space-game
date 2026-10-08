// include/engine/subsystem.h -- the engine lifecycle contract.
//
// Every engine (combat, battle, effect, ai, loot, progression, audio, ...)
// implements the same four-phase lifecycle so the bootloader can bring them
// up in a fixed order and the loading screen can report progress.
//
//   Init     -- allocate, zero, read config
//   Start    -- begin running (subscribe to events, open devices)
//   Update   -- advance one tick
//   Stop     -- stop running
//   Shutdown -- release everything

#ifndef SBW_ENGINE_SUBSYSTEM_H
#define SBW_ENGINE_SUBSYSTEM_H

enum subsystemid
{
    SYS_CORE = 0,
    SYS_DATA,
    SYS_STATS,
    SYS_EFFECTS,
    SYS_COMBAT,
    SYS_BATTLE,
    SYS_AI,
    SYS_LOOT,
    SYS_PROGRESSION,
    SYS_GALAXY,
    SYS_UI,
    SYS_AUDIO,
    SYS_DB,
    SYS_SCRIPT,
    SYS_COUNT
};

// A named subsystem with the standard lifecycle.
struct subsystem
{
    const char *name;
    subsystemid id;
    int initialized;
    int running;
    int priority;

    // Hooks.  Any may be null.
    int (*init)();
    int (*start)();
    int (*update)(int ticks);
    void (*stop)();
    void (*shutdown)();
};

// Register a subsystem with the kernel.
int kernelRegister(const subsystem &sys);

// Bring every registered subsystem up in priority order.
int kernelInitAll();

// Start every initialized subsystem.
int kernelStartAll();

// Advance every running subsystem by `ticks`.
int kernelUpdateAll(int ticks);

// Stop and shut down everything, in reverse order.
void kernelShutdownAll();

// Look up a subsystem by id or name.  Returns null if unknown.
const subsystem *kernelGet(subsystemid id);
const subsystem *kernelGetByName(const char *name);

// Diagnostics.
int kernelSubsystemCount();
int kernelRunningCount();
void kernelDumpStatus();

#endif /* SBW_ENGINE_SUBSYSTEM_H */

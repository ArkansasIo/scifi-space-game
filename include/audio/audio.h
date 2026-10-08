// include/audio/audio.h -- the audio engine.
//
// Plays sound effects and music cues.  The engine is a *cue dispatcher*:
// game code names a cue (SFX_CANNON_FIRE) and the audio backend decides how
// to render it.  On a terminal build the default backend is the null backend,
// which logs cues and does nothing else, so headless runs stay silent.
//
// Define SBW_ENABLE_AUDIO and link a backend to render for real.

#ifndef SBW_AUDIO_AUDIO_H
#define SBW_AUDIO_AUDIO_H

/* ---------------- sound effect ids ---------------- */

enum soundcue
{
    SFX_NONE = 0,

    /* UI */
    SFX_UI_MOVE,
    SFX_UI_SELECT,
    SFX_UI_BACK,
    SFX_UI_ERROR,
    SFX_UI_CONFIRM,

    /* Weapons */
    SFX_CANNON_FIRE,
    SFX_LASER_FIRE,
    SFX_MISSILE_LAUNCH,
    SFX_RAILGUN_FIRE,
    SFX_PLASMA_FIRE,
    SFX_TORPEDO_LAUNCH,

    /* Impacts and defence */
    SFX_HULL_HIT,
    SFX_SHIELD_HIT,
    SFX_SHIELD_DOWN,
    SFX_CRITICAL_HIT,
    SFX_BLOCK,
    SFX_DODGE,

    /* Powers */
    SFX_ABILITY_CAST,
    SFX_BUFF_APPLIED,
    SFX_DEBUFF_APPLIED,
    SFX_HEAL,

    /* Outcomes */
    SFX_ENEMY_EXPLODE,
    SFX_PLAYER_DAMAGED,
    SFX_VICTORY,
    SFX_DEFEAT,

    /* System */
    SFX_LEVEL_UP,
    SFX_LOOT_DROP,
    SFX_RARE_DROP,
    SFX_LEGENDARY_DROP,
    SFX_BOSS_PHASE,
    SFX_ALARM,
    SFX_WARP,
    SFX_DOCKING,

    SFX_COUNT
};

/* ---------------- music ids ---------------- */

enum musiccue
{
    MUS_NONE = 0,
    MUS_TITLE,
    MUS_AMBIENT_SPACE,
    MUS_COMBAT,
    MUS_BOSS,
    MUS_VICTORY,
    MUS_DEFEAT,
    MUS_EMPIRE,
    MUS_COUNT
};

/* ---------------- channel / bus ---------------- */

enum audiobus
{
    BUS_MASTER = 0,
    BUS_SFX,
    BUS_MUSIC,
    BUS_UI,
    BUS_AMBIENCE,
    BUS_COUNT
};

/* ---------------- settings ---------------- */

struct audiosettings
{
    int masterVolume;   /* 0..100 */
    int sfxVolume;      /* 0..100 */
    int musicVolume;    /* 0..100 */
    int uiVolume;       /* 0..100 */
    int ambienceVolume; /* 0..100 */
    int muted;
    int enabled;
};

/* ---------------- lifecycle ---------------- */

// Bring the audio engine up.  Returns 1 on success.
int audioInit();

// Start the backend (open the device).
int audioStart();

// Advance internal state: fade music, flush one-shots.
// Returns 1 while the audio subsystem is running, 0 otherwise.
int audioUpdate(int ticks);

// Stop playback.
void audioStop();
// Release everything.
void audioShutdown();

// Is a real backend running?
int audioIsEnabled();

// Last error from any audio* call (never null).
const char *audioLastError();

/* ---------------- settings ---------------- */

void audioDefaultSettings(audiosettings &settings);
void audioSetSettings(const audiosettings &settings);
const audiosettings &audioGetSettings();
void audioSetBusVolume(audiobus bus, int volume);
int audioGetBusVolume(audiobus bus);
void audioSetMuted(int muted);
int audioIsMuted();

// Effective volume of a cue after its bus and the master are applied.
int audioEffectiveVolume(audiobus bus);

/* ---------------- playback ---------------- */

// Play a one-shot effect.  `pan` is -100 (left) .. +100 (right).
int audioPlaySFX(soundcue cue, int pan);

// Play a one-shot at full centre pan.
int audioPlaySFX(soundcue cue);

// Start a looping effect (engine hum, alarm).  Returns a handle, or -1.
int audioPlayLoop(soundcue cue, int pan);
void audioStopLoop(int handle);

// Play a music track, crossfading from whatever is playing.
int audioPlayMusic(musiccue cue);
musiccue audioCurrentMusic();
void audioStopMusic();

// Fade out in the given number of ticks.
void audioFadeOut(int ticks);

/* ---------------- registry ---------------- */

// The file a cue maps to, from the sound manifest.
const char *audioCueFile(soundcue cue);
const char *audioCueName(soundcue cue);
const char *audioMusicName(musiccue cue);
const char *audioBusName(audiobus bus);

// Load the cue -> file manifest.  Falls back to built-in defaults.
void audioLoadManifest(const char *path);

// Number of cues that have a resolvable file.
int audioResolvedCueCount();

/* ---------------- diagnostics ---------------- */

// Print the cue table with volume and resolution state.
void audioDumpManifest();

// How many times a cue has been played this session.
int audioPlayCount(soundcue cue);

// Reset the play counters.
void audioResetStats();

#endif /* SBW_AUDIO_AUDIO_H */

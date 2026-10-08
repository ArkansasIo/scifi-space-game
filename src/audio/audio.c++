// src/audio/audio.c++ -- the audio engine.
//
// A cue dispatcher.  Game code names a cue; this module resolves it to a file,
// applies bus volumes, and hands it to the backend.  The default backend is
// the null backend: it book-keeps everything (handles, play counts, current
// music) and logs, so headless and test runs behave identically to normal
// play, minus sound.
//
// Define SBW_ENABLE_AUDIO and link a backend to render for real.

#include "spacebattlerpg.h"
#include "audio/audio.h"
#include "audio/manifest.h"

/* ---------------- internal state ---------------- */

static int g_audioRunning = 0;
static char g_audioError[MAXLEN + 1];

static audiosettings g_settings;
static musiccue g_currentMusic = MUS_NONE;
static int g_musicFadeTicks = 0;
static int g_nextLoopHandle = 1;

static int g_playCounts[SFX_COUNT];

// Overridden cue files, from a loaded manifest.  A null entry means "use the
// built-in default".
static const char *g_sfxOverride[SFX_COUNT];
static const char *g_musicOverride[MUS_COUNT];

static void audioSetError(const char *message)
{
    strncpy(g_audioError, message ? message : "", sizeof(g_audioError) - 1);
    g_audioError[sizeof(g_audioError) - 1] = '\0';
}

static int audioClampVolume(int volume)
{
    if (volume < 0)
        return 0;
    if (volume > 100)
        return 100;
    return volume;
}

/* ---------------- lifecycle ---------------- */

int audioInit()
{
    audioDefaultSettings(g_settings);

    for (int i = 0; i < SFX_COUNT; ++i)
    {
        g_sfxOverride[i] = 0;
        g_playCounts[i] = 0;
    }

    for (int i = 0; i < MUS_COUNT; ++i)
        g_musicOverride[i] = 0;

    g_currentMusic = MUS_NONE;
    g_musicFadeTicks = 0;
    g_nextLoopHandle = 1;

#ifdef SBW_ENABLE_AUDIO
    g_audioRunning = 1;
#else
    g_audioRunning = 0;
#endif

    audioSetError("");
    return g_audioRunning;
}

int audioStart()
{
    if (!g_settings.enabled)
    {
        audioSetError("audio disabled in settings");
        return 0;
    }

    audioSetError("");
    return g_audioRunning;
}

int audioUpdate(int ticks)
{
    if (g_musicFadeTicks > 0)
    {
        g_musicFadeTicks -= ticks;

        if (g_musicFadeTicks <= 0)
        {
            g_musicFadeTicks = 0;
            g_currentMusic = MUS_NONE;
        }
    }

    return g_audioRunning;
}

void audioStop()
{
    g_currentMusic = MUS_NONE;
    g_musicFadeTicks = 0;
}

void audioShutdown()
{
    audioStop();
    g_audioRunning = 0;
}

int audioIsEnabled()
{
    return g_audioRunning && !g_settings.muted && g_settings.enabled;
}

const char *audioLastError()
{
    return g_audioError;
}

/* ---------------- settings ---------------- */

void audioDefaultSettings(audiosettings &settings)
{
    settings.masterVolume = BUS_DEFAULT_VOLUME[BUS_MASTER];
    settings.sfxVolume = BUS_DEFAULT_VOLUME[BUS_SFX];
    settings.musicVolume = BUS_DEFAULT_VOLUME[BUS_MUSIC];
    settings.uiVolume = BUS_DEFAULT_VOLUME[BUS_UI];
    settings.ambienceVolume = BUS_DEFAULT_VOLUME[BUS_AMBIENCE];
    settings.muted = 0;
    settings.enabled = 1;
}

void audioSetSettings(const audiosettings &settings)
{
    g_settings = settings;

    g_settings.masterVolume = audioClampVolume(g_settings.masterVolume);
    g_settings.sfxVolume = audioClampVolume(g_settings.sfxVolume);
    g_settings.musicVolume = audioClampVolume(g_settings.musicVolume);
    g_settings.uiVolume = audioClampVolume(g_settings.uiVolume);
    g_settings.ambienceVolume = audioClampVolume(g_settings.ambienceVolume);
}

const audiosettings &audioGetSettings()
{
    return g_settings;
}

void audioSetBusVolume(audiobus bus, int volume)
{
    volume = audioClampVolume(volume);

    switch (bus)
    {
    case BUS_MASTER:
        g_settings.masterVolume = volume;
        break;
    case BUS_SFX:
        g_settings.sfxVolume = volume;
        break;
    case BUS_MUSIC:
        g_settings.musicVolume = volume;
        break;
    case BUS_UI:
        g_settings.uiVolume = volume;
        break;
    case BUS_AMBIENCE:
        g_settings.ambienceVolume = volume;
        break;
    default:
        break;
    }
}

int audioGetBusVolume(audiobus bus)
{
    switch (bus)
    {
    case BUS_MASTER:
        return g_settings.masterVolume;
    case BUS_SFX:
        return g_settings.sfxVolume;
    case BUS_MUSIC:
        return g_settings.musicVolume;
    case BUS_UI:
        return g_settings.uiVolume;
    case BUS_AMBIENCE:
        return g_settings.ambienceVolume;
    default:
        return 0;
    }
}

void audioSetMuted(int muted)
{
    g_settings.muted = muted ? 1 : 0;
}

int audioIsMuted()
{
    return g_settings.muted;
}

int audioEffectiveVolume(audiobus bus)
{
    if (g_settings.muted || !g_settings.enabled)
        return 0;

    // Master multiplies the bus.  Integer maths, so scale back by 100.
    return (g_settings.masterVolume * audioGetBusVolume(bus)) / 100;
}

/* ---------------- playback ---------------- */

int audioPlaySFX(soundcue cue, int pan)
{
    if (cue <= SFX_NONE || cue >= SFX_COUNT)
        return 0;

    if (pan < -100)
        pan = -100;
    if (pan > 100)
        pan = 100;

    g_playCounts[cue]++;

    if (!audioIsEnabled())
        return 0;

    // Which bus a cue belongs on is a property of the cue's family.
    audiobus bus = BUS_SFX;
    if (cue >= SFX_UI_MOVE && cue <= SFX_UI_CONFIRM)
        bus = BUS_UI;

    if (audioEffectiveVolume(bus) <= 0)
        return 0;

    /* Real implementation: resolve audioCueFile(cue), mix at pan, play. */
    audioSetError("");
    return 1;
}

int audioPlaySFX(soundcue cue)
{
    return audioPlaySFX(cue, 0);
}

int audioPlayLoop(soundcue cue, int pan)
{
    (void)pan;  // a real backend would mix the loop at this pan

    if (cue <= SFX_NONE || cue >= SFX_COUNT)
        return -1;

    if (!audioIsEnabled())
        return -1;

    int handle = g_nextLoopHandle;
    g_nextLoopHandle++;

    /* Real implementation: keep the voice alive until audioStopLoop(). */
    return handle;
}

void audioStopLoop(int handle)
{
    if (handle <= 0)
        return;

    /* Real implementation: stop and release the looping voice. */
}

int audioPlayMusic(musiccue cue)
{
    if (cue < MUS_NONE || cue >= MUS_COUNT)
        return 0;

    if (cue == g_currentMusic)
        return 1;  // already playing

    g_currentMusic = cue;
    g_musicFadeTicks = 0;

    if (!audioIsEnabled() || cue == MUS_NONE)
        return 0;

    /* Real implementation: crossfade the previous track into this one. */
    return 1;
}

musiccue audioCurrentMusic()
{
    return g_currentMusic;
}

void audioStopMusic()
{
    g_currentMusic = MUS_NONE;
    g_musicFadeTicks = 0;
}

void audioFadeOut(int ticks)
{
    if (ticks < 0)
        ticks = 0;

    g_musicFadeTicks = ticks;
}

/* ---------------- registry ---------------- */

const char *audioCueFile(soundcue cue)
{
    if (cue <= SFX_NONE || cue >= SFX_COUNT)
        return "";

    if (g_sfxOverride[cue])
        return g_sfxOverride[cue];

    return SFX_FILES[cue];
}

const char *audioCueName(soundcue cue)
{
    if (cue <= SFX_NONE || cue >= SFX_COUNT)
        return "none";

    return SFX_NAMES[cue];
}

const char *audioMusicName(musiccue cue)
{
    if (cue < MUS_NONE || cue >= MUS_COUNT)
        return "none";

    return MUSIC_NAMES[cue];
}

const char *audioBusName(audiobus bus)
{
    if (bus < 0 || bus >= BUS_COUNT)
        return "unknown";

    return BUS_NAMES[bus];
}

void audioLoadManifest(const char *path)
{
    if (!path || path[0] == '\0')
    {
        audioSetError("no manifest path");
        return;
    }

    FILE *f = fopen(path, "rb");
    if (!f)
    {
        audioSetError("manifest not found; using built-in defaults");
        return;
    }

    // A real loader would parse `cue = file` lines here.  The built-in
    // defaults remain in place for anything not overridden.
    char line[MAXLEN];

    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;

        // Deliberately conservative: unknown keys are ignored.
        continue;
    }

    fclose(f);
    audioSetError("");
}

int audioResolvedCueCount()
{
    int resolved = 0;

    for (int i = 1; i < SFX_COUNT; ++i)
    {
        const char *file = audioCueFile((soundcue)i);
        if (file && file[0] != '\0')
            resolved++;
    }

    return resolved;
}

/* ---------------- diagnostics ---------------- */

void audioDumpManifest()
{
    cout << "  Audio cue manifest (" << audioResolvedCueCount() << "/"
         << (SFX_COUNT - 1) << " resolved)" << endl;
    cout << "  Backend: " << (g_audioRunning ? "live" : "null (stub)")
         << endl;
    cout << endl;

    for (int i = 1; i < SFX_COUNT; ++i)
    {
        audiobus bus = BUS_SFX;
        if (i >= SFX_UI_MOVE && i <= SFX_UI_CONFIRM)
            bus = BUS_UI;

        cout << "    " << SFX_NAMES[i];
        for (int pad = (int)strlen(SFX_NAMES[i]); pad < 18; ++pad)
            cout << ' ';

        cout << audioCueFile((soundcue)i)
             << "  [" << audioBusName(bus) << "]" << endl;
    }

    cout << endl;
}

int audioPlayCount(soundcue cue)
{
    if (cue <= SFX_NONE || cue >= SFX_COUNT)
        return 0;

    return g_playCounts[cue];
}

void audioResetStats()
{
    for (int i = 0; i < SFX_COUNT; ++i)
        g_playCounts[i] = 0;
}

// include/audio/manifest.h -- cue name / file mapping.
//
// Generated from the cue enums in include/audio/audio.h.  Each cue resolves to
// a filename under assets/audio/, e.g. SFX_CANNON_FIRE -> "cannon_fire.wav".
//
// The manifest is data, not code: audioLoadManifest() can override any entry
// from assets/audio/manifest.ini without a rebuild.

#ifndef SBW_AUDIO_MANIFEST_H
#define SBW_AUDIO_MANIFEST_H

// Built-in defaults, indexed by soundcue / musiccue.
extern const char *SFX_FILES[SFX_COUNT];
extern const char *SFX_NAMES[SFX_COUNT];
extern const char *MUSIC_FILES[MUS_COUNT];
extern const char *MUSIC_NAMES[MUS_COUNT];
extern const char *BUS_NAMES[BUS_COUNT];
extern const int BUS_DEFAULT_VOLUME[BUS_COUNT];

// Root directory audio files are resolved against.
#define SBW_AUDIO_ROOT "assets/audio/"

#endif /* SBW_AUDIO_MANIFEST_H */

// src/audio/manifest.c++ -- the built-in cue name / file tables.
//
// Every cue in include/audio/audio.h has one row here.  Files are resolved
// against SBW_AUDIO_ROOT (assets/audio/).

#include "spacebattlerpg.h"
#include "audio/audio.h"
#include "audio/manifest.h"

// Index 0 is the "no sound" sentinel.
const char *SFX_FILES[SFX_COUNT] = {
    "", /* SFX_NONE */

    /* UI */
    "ui_move.wav",
    "ui_select.wav",
    "ui_back.wav",
    "ui_error.wav",
    "ui_confirm.wav",

    /* Weapons */
    "cannon_fire.wav",
    "laser_fire.wav",
    "missile_launch.wav",
    "railgun_fire.wav",
    "plasma_fire.wav",
    "torpedo_launch.wav",

    /* Impacts and defence */
    "hull_hit.wav",
    "shield_hit.wav",
    "shield_down.wav",
    "critical_hit.wav",
    "block.wav",
    "dodge.wav",

    /* Powers */
    "ability_cast.wav",
    "buff_applied.wav",
    "debuff_applied.wav",
    "heal.wav",

    /* Outcomes */
    "enemy_explode.wav",
    "player_damaged.wav",
    "victory.wav",
    "defeat.wav",

    /* System */
    "level_up.wav",
    "loot_drop.wav",
    "rare_drop.wav",
    "legendary_drop.wav",
    "boss_phase.wav",
    "alarm.wav",
    "warp.wav",
    "docking.wav"};

const char *SFX_NAMES[SFX_COUNT] = {
    "none",

    "ui_move",
    "ui_select",
    "ui_back",
    "ui_error",
    "ui_confirm",

    "cannon_fire",
    "laser_fire",
    "missile_launch",
    "railgun_fire",
    "plasma_fire",
    "torpedo_launch",

    "hull_hit",
    "shield_hit",
    "shield_down",
    "critical_hit",
    "block",
    "dodge",

    "ability_cast",
    "buff_applied",
    "debuff_applied",
    "heal",

    "enemy_explode",
    "player_damaged",
    "victory",
    "defeat",

    "level_up",
    "loot_drop",
    "rare_drop",
    "legendary_drop",
    "boss_phase",
    "alarm",
    "warp",
    "docking"};

const char *MUSIC_FILES[MUS_COUNT] = {
    "",                      /* MUS_NONE */
    "title_theme.ogg",
    "ambient_space.ogg",
    "combat_theme.ogg",
    "boss_theme.ogg",
    "victory_sting.ogg",
    "defeat_sting.ogg",
    "empire_theme.ogg"};

const char *MUSIC_NAMES[MUS_COUNT] = {
    "none",
    "title",
    "ambient_space",
    "combat",
    "boss",
    "victory",
    "defeat",
    "empire"};

const char *BUS_NAMES[BUS_COUNT] = {
    "master", "sfx", "music", "ui", "ambience"};

// Default bus volumes, out of 100.  Music and ambience sit under the SFX.
const int BUS_DEFAULT_VOLUME[BUS_COUNT] = {
    100, /* master */
    85,  /* sfx */
    60,  /* music */
    90,  /* ui */
    45 /* ambience */};

-- scripts/bosses/ash_tyrant.lua
-- Worked example of a 3-phase raid boss.
--
-- Phases trigger on HP thresholds, each unlocking abilities and (in phase 2)
-- an add spawner.  A soft enrage ramps damage over time; a hard enrage wipes
-- the raid if the fight runs long.  See docs/FRAMEWORK.md s.8.

boss = {
  name = "The Ash Tyrant",
  tier = "raid_boss",
  level = 70,
  hpPool = 2500000,
  tags = "Undead, Titan",

  phases = {
    {
      name = "Cinders",
      hpThreshold = 100,
      timeLimit = 0,
      abilityCount = 4,
      addSpawnerIndex = -1,
      immuneMask = 0,          -- no immunities yet
    },
    {
      name = "Pyroclasm",
      hpThreshold = 60,
      timeLimit = 0,
      abilityCount = 6,
      addSpawnerIndex = 0,     -- summons ember adds
      immuneMask = 0,
    },
    {
      name = "Final Conflagration",
      hpThreshold = 25,
      timeLimit = 0,
      abilityCount = 8,
      addSpawnerIndex = 0,
      immuneMask = 0,          -- still killable, but very fast
    },
  },

  -- Soft enrage: +12% damage every 15 seconds.
  softEnrageRate = 12,
  -- Hard enrage at 8 minutes: instant wipe cast.
  hardEnrageTime = 480,

  lootTableIndex = 0,
  -- Raid bosses scale with player count (docs/FRAMEWORK.md s.10).
  partyScaling = 100,
}

-- Mechanics: a raid-wide AoE pulse, a targeted beam, and a soak mechanic,
-- each gated behind a telegraph longer than 1.5s so it can be countered.
mechanics = {
  { name = "Cinder Pulse",   type = "raid_aoe",    telegraph = 2.0, cooldown = 30 },
  { name = "Ashen Beam",     type = "targeted",    telegraph = 1.8, cooldown = 20 },
  { name = "Ember Soak",     type = "soak",        telegraph = 2.5, cooldown = 45 },
}

bossIndex = registerBoss(boss)

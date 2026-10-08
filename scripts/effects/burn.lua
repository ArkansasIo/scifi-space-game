-- scripts/effects/burn.lua
-- Worked example of a damage-over-time debuff.
--
-- Stack rules, tick rate and the DR category are all declared here, matching
-- include/core/types.h `struct effectdefinition`.

effect = {
  name = "Burn",
  category = "dot",
  damageType = "fire",

  -- Reapplying refreshes the duration instead of adding a stack.
  stackMode = "refresh",
  maxStacks = 1,

  duration = 12,      -- ticks
  magnitude = 300,    -- damage per tick, before mitigation
  tickRate = 1,       -- one tick per game tick

  -- -1 means "not subject to crowd-control diminishing returns".
  drCategory = -1,

  -- Removed by magic cleanses and by a full cleanse.
  cleanse = "magic",

  immunityWindow = 0,
}

effectIndex = registerEffect(effect)

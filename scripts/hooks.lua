-- scripts/hooks.lua
-- Behaviour hooks.  Each event id is documented in scripts/README.md.

-- Combat started: reset per-fight state.
onEvent(SBW_EVENT_COMBAT_START, function(attackerId, defenderId)
  fightTurns = 0
end)

-- Damage dealt: feed telemetry / balance tracking.
onEvent(SBW_EVENT_DAMAGE_DEALT, function(amount, damageType)
  telemetryDamage = (telemetryDamage or 0) + amount
end)

-- Enemy killed: award score, scaled by overkill.
onEvent(SBW_EVENT_ENEMY_KILLED, function(enemyId, overkill)
  score.add(100 + overkill * 2)
end)

-- Loot rolled: log the rarity for drop-rate tuning.
onEvent(SBW_EVENT_LOOT_ROLLED, function(tableIndex, rarity)
  print("loot table " .. tableIndex .. " rolled rarity " .. rarity)
end)

-- Level up: grant the attribute point.
onEvent(SBW_EVENT_LEVEL_UP, function(newLevel)
  attributes.pendingPoints = (attributes.pendingPoints or 0) + 1
end)

-- Boss phase change: announce and log the phase index.
onEvent(SBW_EVENT_BOSS_PHASE, function(bossId, phaseIndex)
  print("phase " .. phaseIndex .. " begins")
end)

-- PvP flag: apply the PvP normalization context.
onEvent(SBW_EVENT_PVP_FLAG, function(playerId, flagged)
  pvpEnabled = (flagged ~= 0)
end)

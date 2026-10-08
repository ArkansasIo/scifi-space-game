-- scripts/items/void_lance.lua
-- Second worked example: a PvP-oriented weapon.
--
-- Note how the PvP table leads with counterplay stats (shield penetration,
-- resource drain) rather than raw damage, and how the PvE table is the one
-- carrying the big numbers.

item = {
  name = "Void Lance",
  family = "weapon",
  slot = "main_hand",
  rarity = "legendary",
  requiredLevel = 60,
  bindRule = "bind_on_equip",

  pve = {
    { label = "ATK",        raw = 1450, weight = 100 },
    { label = "Void Damage", raw = 180, weight = 140 },
    { label = "Armor Pen",  raw = 12,   weight = 200 },
    { label = "Execute Dmg", raw = 15,  weight = 175 },
  },

  pvp = {
    { label = "ATK",         raw = 700, weight = 100 },
    { label = "Shield Pen",  raw = 15,  weight = 220 },
    { label = "Resource Drain", raw = 10, weight = 180 },
    { label = "CC Strength", raw = 6,   weight = 300 },
  },

  procs = {},

  -- Long-duration CC is capped hard in PvP (docs/FRAMEWORK.md s.17).
  pvpCCStrengthCapped = true,
  pvpCritDamageCapPercent = 60,
}

itemIndex = registerItem(item)

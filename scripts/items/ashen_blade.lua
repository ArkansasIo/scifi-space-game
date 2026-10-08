-- scripts/items/ashen_blade.lua
-- Worked example of the PvE / PvP split described in docs/FRAMEWORK.md s.17.
--
-- The same weapon carries two stat tables.  The PvE table keeps the full
-- power fantasy; the PvP table is normalized and dampened so it cannot
-- burst a player down.

item = {
  name = "Ashen Blade",
  family = "weapon",
  slot = "main_hand",
  rarity = "epic",
  requiredLevel = 40,
  bindRule = "bind_on_pickup",

  -- PvE table: high raw damage, elemental scaling, boss bonus.
  pve = {
    { label = "ATK",        raw = 1200, weight = 100 },
    { label = "Fire Damage", raw = 120,  weight = 130 },
    { label = "STR",        raw = 50,    weight = 100 },
    { label = "Crit Chance", raw = 5,    weight = 250 },
  },

  -- PvP table: normalized damage, anti-heal, capped burn.
  pvp = {
    { label = "ATK",        raw = 650, weight = 100 },
    { label = "PvP Power",  raw = 10,  weight = 300 },
    { label = "Anti-Heal",  raw = 20,  weight = 150 },
  },

  -- Procs: magnitude * uptime% * impactWeight%.
  procs = {
    { name = "Burn on hit", magnitude = 300, uptime = 25, impact = 120 },
  },

  -- Boss-specific bonus, PvE only.
  bossBonusPercent = 10,
  pvpBurnCapped = true,

  -- Procs are disabled outright in competitive play.
  pvpProcsEnabled = false,
}

-- Registering the item pushes it into itemArray[] and returns its index.
itemIndex = registerItem(item)

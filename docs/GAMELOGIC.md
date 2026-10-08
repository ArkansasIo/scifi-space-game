# Game Logic Reference

**Modules:** `src/game/` — combat, threat, encounter, loot, ai, progression, galaxy, gamelogic, turn, scheduler
**Status:** implemented

---

## 1. Purpose

Where `library/` holds pure maths with no state, `game/` holds the rules that
need to know about the world: who hates whom, what drops, what happens next.
This is the function-by-function reference.

---

## 2. `game/combat.c++` — Resolution

The pure combat rules. Nothing here reads or writes global state, so every
function is directly testable.

### 2.1 Attack resolution

```c
combatevent resolveAttack(const statblock &attacker,
                          const statblock &defender,
                          int rawDamage, damagetype type,
                          combatcontext context);
```

Four stages, in order:

```
1. Hit roll     Accuracy / (Accuracy + Evasion), clamped 5%..95%
2. Crit roll    clamped to 40% PvP / 70% PvE
3. Mitigation   Armor / (Armor + K × AttackerLevel)
4. Resistance   percent resist + vulnerability
```

`combatevent` reports every intermediate so the UI can say *why*: `dodged`,
`crit`, `mitigated`, and the final `damage`.

**True damage bypasses the entire chain** — the one type that does.

### 2.2 Healing

```c
int resolveHeal(const statblock &healer, statblock &target,
                int healPower, int skillCoef, combatcontext context);
```

Healing carries a **−35% penalty in PvP** to prevent stall comps. Clamped to
the target's max HP.

### 2.3 Damage over time

```c
int resolveDoTTick(const statblock &source, const statblock &target,
                   int dotPower, int skillCoef, damagetype type);
```

Uses the source's **DoT power**, not raw attack — so a burst build and a DoT
build scale differently. Elemental resistance still applies.

A DoT that lands always ticks for at least 1, so a resisted DoT is never
silently useless.

### 2.4 Single exchange

```c
void runCombatTurn(statblock &player, statblock &enemy);
```

Player strikes, then enemy strikes if still alive. Used by callers driving
combat themselves; the engine uses the full state machine instead.

---

## 3. `game/threat.c++` — Aggro

```c
int computeThreat(int damage, int healing, int flatThreat,
                  int dmgWeight, int healWeight);
```

```
Threat = Damage × D_Weight + Healing × H_Weight + FlatThreat
```

Healing is deliberately weighted **below** damage, so a healer cannot
accidentally top the threat table by doing their job.

### 3.1 Stances

```c
int addThreat(threatentry table[], int count, int entityId,
              int amount, int stancePercent);
```

| Stance | Multiplier |
| --- | --- |
| Tank | +200–400% |
| DPS | 0–100% |
| Stealth | −100% (generates nothing) |

### 3.2 Taunts

```c
int tauntThreat(threatentry table[], int count, int entityId, int ticks);
```

Sets threat to `top + 1` **and** forces the target for N ticks. Both, because
a taunt that only grabbed aggro for one tick would immediately lose it again
to whoever was already ahead.

```c
int pickTarget(const threatentry table[], int count);
```

A live forced target wins outright; otherwise it's highest threat.

### 3.3 Other helpers

```c
void clearThreatTable(threatentry table[], int count);
int  findThreatIndex(const threatentry table[], int count, int entityId);
int  highestThreatIndex(const threatentry table[], int count);
void decayThreat(threatentry table[], int count, int percent);
void tickThreat(threatentry table[], int count);
```

`decayThreat` is applied by callers on a timer, letting the table settle
during a lull rather than snapping.

---

## 4. `game/encounter.c++` — Generation

```c
void buildEncounter(encounterinstance &enc, int templateIndex,
                    int playerLevel, int partySize, enemytier tier);
```

Fills an instance from a template: trash → casters → minibosses → boss, each
scaled to the party:

```
HP  = (100 + level × 10) × archetypeHPMultiplier / 100
DMG = (10 + level)        × archetypeDamageMultiplier / 100
```

Then computes the encounter's power budget:

```
EncounterPB = PlayerPower × DifficultyFactor
```

### 4.1 Builders

| Function | Produces |
| --- | --- |
| `buildOpenWorldPack()` | 3 trash + 1 caster + patrol |
| `buildDungeonRoom(seed)` | Hazard / miniboss / puzzle / gauntlet, chosen by seed |
| `buildRaidEncounter(boss, partySize)` | Boss + adds, scaled to the party |

`buildDungeonRoom` selects by `seed % 4`, so a run's room sequence is
reproducible from its seed.

### 4.2 Boss helpers

Defined once in `data/bosses.c++` (they read the boss table):

```c
const bossphase *currentBossPhase(const bossdata &boss, int hpPercent);
int bossShouldEnrage(const bossdata &boss, int elapsedTicks);
int softEnrageBonus(const bossdata &boss, int elapsedTicks);
```

---

## 5. `game/loot.c++` — Drops

```c
int rollLoot(const loottable &table, int difficultyTier, int partySize,
             int luck, lootresult out[], int outMax);
```

Rolls `entryCount + (partySize − 1)` times — one per player plus the table's
base — and walks the weight list for each.

### 5.1 Rarity

```c
gearrarity rollRarity(const loottable &table, int luck);
```

| Roll (0..999, after luck) | Rarity |
| --- | --- |
| ≤ 5 | Mythic |
| ≤ 25 | Legendary |
| ≤ 100 | Epic |
| ≤ 300 | Rare |
| ≤ 600 | Uncommon |
| else | Common |

The **pity timer is checked first**, before the roll. After `pityTimer`
consecutive bad rolls the result is forced to Rare or better. Bad-luck
protection outranks luck.

### 5.2 Bounded luck

```c
int boundedLuck(int rawLuck, int cap);   // default cap 50
```

Hard-capped, so a stacked bonus can never break the economy. `rollLoot` clamps
before use.

### 5.3 Reward multiplier

```c
int rewardMultiplier(int difficultyTier, enemytier tier,
                     int clearTimePercent, int noDeath);
```

```
multiplier = (100 + difficultyTier × 20)
           × tierMultiplier(tier) / 100
           × 100 / clearTimePercent
           + (noDeath ? 15 : 0)
```

Reward scales with **how** you won, not just that you did.

---

## 6. `game/ai.c++` — Enemy Decisions

### 6.1 State machine

```c
void tickAIState(aibrain &brain, int distanceToTarget,
                 int targetHpPercent, int alliesAlive);
```

```
IDLE → ALERT → ENGAGE → COMBAT → RETREAT → RESET
```

Triggers: a target inside `aggroRadius` escalates; losing all allies for 20
ticks starts a retreat; breaking line of sight resets.

### 6.2 Utility scoring

```c
void scoreAbility(abilitycandidate &ability, int distance, int preferredRange,
                  int targetHpPercent, int cooldownReady, int phaseAllowed,
                  int playerCluster, int threatIsTank, int currentTick);
```

Six weighted inputs:

| Input | Weight | Rationale |
| --- | --- | --- |
| Distance fit | up to 100 | Prefers fighting at its ideal range |
| Target HP | 60 / 20 | Below 30% HP is an execute window |
| Cooldown ready | 40 / 0 | |
| Phase allowed | 30 / 0 | |
| Player clustering | 35 / 10 | Clustered players favour AoE |
| Threat target | 25 / 15 | The tank is the expected target |
| Recency | up to 100 | Rewards abilities not used recently |

### 6.3 No-repeat lockout

```c
int  abilityLockedOut(const aibrain &brain, int abilityId, int currentTick);
void markAbilityUsed(aibrain &brain, int abilityId, int currentTick);
```

A 3-tick lockout stops the AI spamming one ability, which is the single most
common "the AI feels broken" complaint.

---

## 7. `game/progression.c++` — Growth

```c
int grantExperience(player &who, int amount);   // returns levels gained
```

```
xpForLevel(level) = 100 + level² × 10
```

The curve is **super-linear** (level²), so each level costs meaningfully more
than the last without the exponential blowup of a real MMO.

Level growth feeds straight back into the ship:

```
power += 5;  sif += 3;  health += 5;
attackpower = power × 2;  defencepower = sif × 2;
```

### 7.1 Other tracks

| Function | Measures |
| --- | --- |
| `commanderRank(fleetVictories)` | 1–10 from fleet wins |
| `reputationTier(reputation)` | 0 hostile → 5 exalted |
| `empireRank(systemsControlled)` | 1–6 from territory |
| `leadingVictory(faction, systems, tech)` | Closest victory path |

`leadingVictory()` picks whichever path the empire's attributes favour most —
so the UI can tell a player what they're actually heading toward.

### 7.2 Research

```c
int researchTime(int techCost, int scienceOutput, int scientistSkill);
```

```
ResearchTime = TechCost / (ScienceOutput × ScientistSkill)
```

---

## 8. `game/galaxy.c++` — The 4X Layer

```c
void initializeGalaxy();          // seed sectors, systems, planets
void rollPlanet(planet &p, const char *name, int systemTier);
void tickEmpireEconomy();         // per-turn production
```

### 8.1 Diplomacy and trade

```c
int diplomaticScore(const faction &who, int reputation, int tradeValue,
                    int sharedEnemies);
int tradeIncome(int tradeValue, int routeSecurity, int distanceModifier);
```

```
DiplomaticScore = Influence + Reputation + TradeValue + SharedEnemies
TradeIncome     = TradeValue × RouteSecurity% × DistanceModifier%
```

Shared enemies counting *positively* is the alliance-building incentive.

### 8.2 Planets

| Function | Output |
| --- | --- |
| `planetCreditOutput(p)` | `resources × 2 + stability / 10` |
| `planetMineralOutput(p)` | `resources × 3` |
| `planetResearchOutput(p)` | `size + habitability / 2 + buildings` |

### 8.3 Fleets

```c
void clearFleet(fleet &f);
void recalcFleet(fleet &f);
int  fleetSupplyUse(const fleet &f);
int  fleetWithinCommandCapacity(const fleet &f);
```

---

## 9. `game/gamelogic.c++` — Integration

The module `main.c++` talks to. It owns the framework boot sequence, the
feature registry and the summary screens.

### 9.1 Boot

```c
void initializeFramework()
{
    initializeFrameworkArrays();   // every data table
    initializeGalaxy();            // the map

    kernelRegister(g_audioSys);    // priority 10
    kernelRegister(g_effectSys);   // 15
    kernelRegister(g_combatSys);   // 20
    kernelRegister(g_battleSys);   // 25
    kernelRegister(g_aiSys);       // 30
    kernelRegister(g_lootSys);     // 35
    kernelRegister(g_progSys);     // 40

    kernelInitAll();
    kernelStartAll();
}
```

Order matches the priority table in `ARCHITECTURE.md` §3.

### 9.2 Summary screens

| Function | Shows |
| --- | --- |
| `showStatSheet(stats)` | Attributes, derived, combat |
| `showPlayerSummary()` | Character panel + derived values |
| `showActiveEffects(list, count)` | Live buffs/debuffs with stacks |
| `showItemTooltip(index)` | **PvE and PvP tables side by side** |
| `showEncounter(enc)` | Unit roster with scaled stats |
| `showBossRoster()` | Every boss and its phase plan |
| `showGalaxyMap()` | Systems, planets, stations |
| `showEmpireDashboard()` | Attributes, resources, rank |

`showItemTooltip` showing both tables at once is the clearest expression of
the project's core rule: *PvE and PvP never share a balance curve.*

### 9.3 Feature registry

```c
const char *featureDescription(int featureId);
int featureCount();          // 20 features
```

A machine-readable inventory of what the engine supports, used by the help
screen and by the `.md` docs to stay honest about scope.

---

## 10. Turn and Scheduler

See `TURNS.md` and `SCHEDULER.md` — the turn system runs the phase machine,
the scheduler fires jobs on their own cadence within those phases.

---

## 11. Function Index

| Module | Public functions |
| --- | --- |
| `combat.c++` | 5 |
| `threat.c++` | 8 |
| `encounter.c++` | 5 |
| `loot.c++` | 5 |
| `ai.c++` | 6 |
| `progression.c++` | 8 |
| `galaxy.c++` | 14 |
| `gamelogic.c++` | 12 |
| `turn.c++` | 18 |
| `scheduler.c++` | 18 |

---

## 12. Open Questions

- **Threat decay timing.** `decayThreat` exists but nothing calls it on a
  schedule; it belongs in a `TPHASE_MAINTENANCE` step.
- **Encounter difficulty.** `powerBudget` is computed but never compared
  against the actual generated units, so an over-budget encounter isn't caught.
- **Faction expansion.** `ownerFaction` is set at generation and never
  updated — the 4X layer is seeded, not simulated.

// include/game/galaxy.h -- the 4X layer: sectors, systems, planets, trade
// and diplomacy.  See docs/FRAMEWORK.md sections 22.2 - 22.10.

#ifndef SBW_GAME_GALAXY_H
#define SBW_GAME_GALAXY_H

// Clear a star system record.
void clearStarSystem(starsystem &sys);

// Seed the galaxy map (sectors -> systems -> planets).
void initializeGalaxy();

// Faction helpers.
int findFaction(const char *name);

// DiplomaticScore = Influence + Reputation + TradeValue + SharedEnemies
int diplomaticScore(const faction &who, int reputation, int tradeValue,
                    int sharedEnemies);

// TradeIncome = TradeValue * RouteSecurity% * DistanceModifier%
int tradeIncome(int tradeValue, int routeSecurity, int distanceModifier);

// Build a planet record with the given strategic roll.
void rollPlanet(planet &p, const char *name, int systemTier);

// Planet output helpers.
int planetCreditOutput(const planet &p);
int planetMineralOutput(const planet &p);
int planetResearchOutput(const planet &p);

// Add a resource bundle into a stockpile.
void addStockpile(stockpile &into, const stockpile &amount);

// Empire-wide production for one turn (adds into empireStorage).
void tickEmpireEconomy();

// Empire strategic priority: Threat + Need + Opportunity + Risk.
int empirePriority(const faction &ai, int threatLevel, int resourceNeed,
                   int expansionOpportunity, int diplomaticRisk);

// ---- fleet helpers ----

// Zero a fleet.
void clearFleet(fleet &f);

// Recompute supply use and fleet power from the ships.
void recalcFleet(fleet &f);

// Total supply consumed by a fleet.
int fleetSupplyUse(const fleet &f);

// True if the fleet is inside its flag capacity.
int fleetWithinCommandCapacity(const fleet &f);

#endif /* SBW_GAME_GALAXY_H */

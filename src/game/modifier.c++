// Modifier.c++ -- the mission modifier (skull) list.

#include "spacebattlerpg.h"

struct modifier
{
    const char *name;
    const char *description;
};

static const modifier modifiers[] = {
    {"Airborne", "Players deal more damage while in the air."},
    {"Angry", "Minions of the Darkness deal more damage."},
    {"Berserk", "Minions of the Darkness won't flinch, even after massive damage."},
    {"Arc Burn", "Arc Damage from any source is greatly increased."},
    {"Brawler", "Guardian melee damage is greatly increased."},
    {"Catapult", "Grenade recharge rate is greatly increased."},
    {"Chaff", "Player radar is disabled."},
    {"Daybreak", "Reduces cooldown of all abilities. Epic is inherent."},
    {"Epic", "Heavily shielded and highly aggressive enemies appear in great numbers."},
    {"Exposure", "Guardian shields are increased but do not replenish."},
    {"Fresh Troops", "Some enemy squads have been fortified with additional reinforcements."},
    {"Grounded", "Players take more damage while airborne."},
    {"Heroic", "Enemies appear in greater numbers and are more aggressive."},
    {"Juggler", "No ammo drops for your equipped weapon."},
    {"Ironclad", "More enemies have shields."},
    {"Lightswitch", "Minions of the Darkness deal much more melee damage."},
    {"Match Game", "Enemy shields are resistant to all unmatched elemental damage."},
    {"Nightfall", "If all players die, the fireteam will be returned to orbit."},
    {"Small Arms", "Primary Weapon damage is favored."},
    {"Solar Burn", "Solar Damage from any source is greatly increased."},
    {"Specialist", "Secondary Weapon damage is favored."},
    {"Trickle", "Recharge of abilities is significantly reduced."},
    {"Void Burn", "Void Damage from any source is greatly increased."}};

void initializeModifier()
{
    const int count = (int)(sizeof(modifiers) / sizeof(modifiers[0]));
    int index = rand() % count;

    cout << "\nMission modifier: " << modifiers[index].name
         << " - " << modifiers[index].description << "\n"
         << endl;
}
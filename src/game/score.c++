// Targetscore.c++ -- tracks and reports the run's score.

#include "spacebattlerpg.h"

void initializeTargetscore()
{
    targetScore.score = 0;
    targetScore.actions_taken = 0;
    targetScore.damage_taken = 0;
    targetScore.kills = 0;
    targetScore.damage_done = 0;
}

void showTargetscore()
{
    cout << "\n===== TARGET SCORE =====" << endl;
    cout << "Score:          " << targetScore.score << endl;
    cout << "Kills:          " << targetScore.kills << endl;
    cout << "Actions taken:  " << targetScore.actions_taken << endl;
    cout << "Damage done:    " << targetScore.damage_done << endl;
    cout << "Damage taken:   " << targetScore.damage_taken << endl;
    cout << "========================\n"
         << endl;
}
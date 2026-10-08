// campaign.c++ -- the campaign / save slot menu.

#include "spacebattlerpg.h"

void initializecampaign()
{
    cout << "\n=========== CAMPAIGN ===========\n";
    cout << "1 - Judgment (new game)\n";
    cout << "2 - Aftermath (new game)\n";
    cout << "3 - Continue game\n";

    int choice = readInt("Pick your campaign: ");

    switch (choice)
    {
    case 1:
        cout << "Starting a new Judgment campaign...\n";
        break;
    case 2:
        cout << "Starting a new Aftermath campaign...\n";
        break;
    case 3:
        cout << "Continuing saved game...\n";
        break;
    default:
        cout << "Error - Invalid input; defaulting to Judgment.\n";
        break;
    }
}
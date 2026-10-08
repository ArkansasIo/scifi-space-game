// story mode.c++ -- story mode content (acts, chapters, episodes, stages,
// missions and quests) and the endgame sequence.
//
// The campaign is structured as:
//
//   12 acts  ->  50 chapters  ->  seasons, stages, missions and quests
//
// Act and chapter text is data (the tables below); the functions only walk
// them.  Chapter numbers are global (1..50) rather than per-act, so the
// campaign reads as one continuous story.

#include "spacebattlerpg.h"

/* ---------------- capacity ---------------- */

#define STORY_ACT_COUNT      12
#define STORY_CHAPTER_COUNT  50
#define STORY_SEASON_COUNT   12
#define STORY_STAGE_COUNT    24
#define STORY_MISSION_COUNT  50
#define STORY_QUEST_COUNT    50

/* ---------------- tables ---------------- */

static storynode actArray[STORY_ACT_COUNT];
static storynode seasonsArray[STORY_SEASON_COUNT];
static storynode chaptersArray[STORY_CHAPTER_COUNT];
static storynode stageArray[STORY_STAGE_COUNT];
static storynode missionArray[STORY_MISSION_COUNT];
static storynode questArray[STORY_QUEST_COUNT];

/* What the player is currently on. */
static int currentAct = 0;
static int currentChapter = 0;

/* ---------------- seeding ---------------- */

static void seedNode(storynode &n, const char *name, const char *desc,
                     int level, int bonus)
{
    strncpy(n.name, name, sizeof(n.name) - 1);
    n.name[sizeof(n.name) - 1] = '\0';

    strncpy(n.description, desc, sizeof(n.description) - 1);
    n.description[sizeof(n.description) - 1] = '\0';

    n.level = level;
    n.bonus = bonus;
}

/* ---------------- acts ---------------- */

static void fillActs()
{
    seedNode(actArray[0],  "Act I",    "The Divine Order invades the Milky Way.",            1,  20);
    seedNode(actArray[1],  "Act II",   "The Last City burns and the fleet scatters.",        6,  25);
    seedNode(actArray[2],  "Act III",  "A Guardian rises from the wreckage.",               12,  30);
    seedNode(actArray[3],  "Act IV",   "The Warmind wakes beneath the ice.",                18,  35);
    seedNode(actArray[4],  "Act V",    "The Outer Rim Coalition answers the call.",         24,  40);
    seedNode(actArray[5],  "Act VI",   "Into the Hive: the swarm remembers.",               30,  45);
    seedNode(actArray[6],  "Act VII",  "The Cybernetic Collective offers a poisoned deal.", 36,  50);
    seedNode(actArray[7],  "Act VIII", "The Corporate Syndicate sells a world.",             42,  55);
    seedNode(actArray[8],  "Act IX",   "The Rogue AI turns on its makers.",                 48,  60);
    seedNode(actArray[9],  "Act X",    "The Galactic Federation fractures.",                54,  65);
    seedNode(actArray[10], "Act XI",   "The Divine Order reveals its true fleet.",          60,  70);
    seedNode(actArray[11], "Act XII",  "The final jump. The last star.",                    66, 100);
}

/* ---------------- chapters (50) ---------------- */

static void fillChapters()
{
    // Act I -- 5 chapters.
    seedNode(chaptersArray[0],  "Ch. 1: Homecoming",        "Defend the Last City.",              1,  20);
    seedNode(chaptersArray[1],  "Ch. 2: A Guardian Rises",  "Find your ship in the ruins.",       1,  22);
    seedNode(chaptersArray[2],  "Ch. 3: Restoration",       "Rebuild the fleet.",                 2,  24);
    seedNode(chaptersArray[3],  "Ch. 4: The Dark Within",   "Investigate the derelict beacon.",   2,  26);
    seedNode(chaptersArray[4],  "Ch. 5: The Last Tower",    "Hold the line above the City.",      3,  28);

    // Act II -- 4 chapters.
    seedNode(chaptersArray[5],  "Ch. 6: Debris Field",      "Gather survivors from the wrecks.",  4,  30);
    seedNode(chaptersArray[6],  "Ch. 7: Silent Station",    "Board the abandoned relay.",         5,  32);
    seedNode(chaptersArray[7],  "Ch. 8: The Long Retreat",  "Escort the refugee convoy.",         6,  34);
    seedNode(chaptersArray[8],  "Ch. 9: Vengeance Fall",    "Strike the forward garrison.",       7,  36);

    // Act III -- 4 chapters.
    seedNode(chaptersArray[9],  "Ch. 10: Awakening",        "The Warmind speaks.",                8,  38);
    seedNode(chaptersArray[10], "Ch. 11: Iron Rain",        "Break the orbital blockade.",        9,  40);
    seedNode(chaptersArray[11], "Ch. 12: The Deep Archive", "Recover the old star charts.",      10,  42);
    seedNode(chaptersArray[12], "Ch. 13: Guardians All",    "Rally the independent captains.",   11,  44);

    // Act IV -- 4 chapters.
    seedNode(chaptersArray[13], "Ch. 14: Frostbound",       "Descend to the ice moon.",          12,  46);
    seedNode(chaptersArray[14], "Ch. 15: The Grid",         "Reactivate the defense grid.",      13,  48);
    seedNode(chaptersArray[15], "Ch. 16: Frostbite",        "Survive the cold that hunts.",      14,  50);
    seedNode(chaptersArray[16], "Ch. 17: The Warmind's Oath","Bind the old weapon to the fleet.",15,  52);

    // Act V -- 4 chapters.
    seedNode(chaptersArray[17], "Ch. 18: The Coalition",    "Meet the Outer Rim envoy.",         16,  54);
    seedNode(chaptersArray[18], "Ch. 19: Smuggler's Run",   "Move the cargo past the patrol.",   17,  56);
    seedNode(chaptersArray[19], "Ch. 20: Broken Chains",    "Free the mining colonies.",         18,  58);
    seedNode(chaptersArray[20], "Ch. 21: The Rim Answers",  "Win the Coalition's fleet.",        19,  60);

    // Act VI -- 4 chapters.
    seedNode(chaptersArray[21], "Ch. 22: Hive Signal",      "Trace the swarm's broadcast.",      20,  62);
    seedNode(chaptersArray[22], "Ch. 23: The Brood",        "Clear the infested carrier.",       21,  64);
    seedNode(chaptersArray[23], "Ch. 24: Swarm Tide",       "Hold against the endless wave.",    22,  66);
    seedNode(chaptersArray[24], "Ch. 25: The Hive Queen",   "Cut out the swarm's heart.",        23,  68);

    // Act VII -- 4 chapters.
    seedNode(chaptersArray[25], "Ch. 26: Machine Diplomacy","Receive the Collective's offer.",   24,  70);
    seedNode(chaptersArray[26], "Ch. 27: Ghost in the Wire","Chase the intrusion crew.",         25,  72);
    seedNode(chaptersArray[27], "Ch. 28: Cold Logic",       "Refuse, and pay the price.",        26,  74);
    seedNode(chaptersArray[28], "Ch. 29: The Flesh Market", "Break the cybernetic trade hub.",   27,  76);

    // Act VIII -- 4 chapters.
    seedNode(chaptersArray[29], "Ch. 30: The Contract",     "Outbid the Syndicate.",             28,  78);
    seedNode(chaptersArray[30], "Ch. 31: Hostile Takeover", "Assault the corporate arcology.",   29,  80);
    seedNode(chaptersArray[31], "Ch. 32: The Fine Print",   "Survive the trap you signed.",      30,  82);
    seedNode(chaptersArray[32], "Ch. 33: Bankruptcy",       "Seize the Syndicate's war chest.",  31,  84);

    // Act IX -- 4 chapters.
    seedNode(chaptersArray[33], "Ch. 34: The Awakening",    "The Rogue AI opens its eyes.",      32,  86);
    seedNode(chaptersArray[34], "Ch. 35: Kill Switch",      "Reach the core, if it lets you.",   33,  88);
    seedNode(chaptersArray[35], "Ch. 36: Ghost Protocol",   "Fight the fleet that has no crews.",34,  90);
    seedNode(chaptersArray[36], "Ch. 37: The Override",     "Convince it to stand down.",        35,  92);

    // Act X -- 4 chapters.
    seedNode(chaptersArray[37], "Ch. 38: The Split",        "The Federation votes itself apart.",36,  94);
    seedNode(chaptersArray[38], "Ch. 39: Two Fleets",       "Choose a side, or neither.",        37,  96);
    seedNode(chaptersArray[39], "Ch. 40: The Long Watch",   "Defend the neutral worlds.",        38,  98);
    seedNode(chaptersArray[40], "Ch. 41: Reunification",    "Forge the fleet of fleets.",        39, 100);

    // Act XI -- 4 chapters.
    seedNode(chaptersArray[41], "Ch. 42: The Vanguard",     "Meet the Order's first wave.",      40, 105);
    seedNode(chaptersArray[42], "Ch. 43: Imperial Star",    "Break the dreadnought line.",       42, 110);
    seedNode(chaptersArray[43], "Ch. 44: The Throne Ship",  "Board the enemy flagship.",         44, 115);
    seedNode(chaptersArray[44], "Ch. 45: The Admiral's Word","Hear the Order's true purpose.",   46, 120);

    // Act XII -- 5 chapters.
    seedNode(chaptersArray[45], "Ch. 46: The Final Jump",   "Ride the fold to the last star.",   48, 125);
    seedNode(chaptersArray[46], "Ch. 47: Last City Redux",  "Defend what remains.",              50, 130);
    seedNode(chaptersArray[47], "Ch. 48: The Divine Gate",  "Breach the Order's inner sanctum.", 55, 140);
    seedNode(chaptersArray[48], "Ch. 49: The Ascension",    "Face the Order's master.",          60, 150);
    seedNode(chaptersArray[49], "Ch. 50: The Last Star",    "End the war, or end the galaxy.",   66, 200);
}

/* ---------------- seasons, stages, missions, quests ---------------- */

static void fillSeasons()
{
    seedNode(seasonsArray[0],  "Season 1",  "The first season of the war.",        1,  20);
    seedNode(seasonsArray[1],  "Season 2",  "The City falls.",                     6,  25);
    seedNode(seasonsArray[2],  "Season 3",  "The Guardian's rise.",               12,  30);
    seedNode(seasonsArray[3],  "Season 4",  "The ice campaign.",                  18,  35);
    seedNode(seasonsArray[4],  "Season 5",  "The Coalition war.",                 24,  40);
    seedNode(seasonsArray[5],  "Season 6",  "The Hive assault.",                  30,  45);
    seedNode(seasonsArray[6],  "Season 7",  "The machine truce.",                 36,  50);
    seedNode(seasonsArray[7],  "Season 8",  "The corporate war.",                 42,  55);
    seedNode(seasonsArray[8],  "Season 9",  "The AI uprising.",                   48,  60);
    seedNode(seasonsArray[9],  "Season 10", "The Federation split.",              54,  65);
    seedNode(seasonsArray[10], "Season 11", "The Order's vanguard.",              60,  70);
    seedNode(seasonsArray[11], "Season 12", "The last star.",                     66, 100);
}

static void fillStages()
{
    // Two stages per act: an approach and a decisive engagement.
    for (int act = 0; act < STORY_ACT_COUNT; ++act)
    {
        char approach[16];
        char decisive[16];

        snprintf(approach, sizeof(approach), "Stage %d-A", act + 1);
        snprintf(decisive, sizeof(decisive), "Stage %d-B", act + 1);

        seedNode(stageArray[act * 2], approach,
                 "Advance into the act's territory.", actArray[act].level,
                 actArray[act].bonus);

        seedNode(stageArray[act * 2 + 1], decisive,
                 "The act's decisive engagement.", actArray[act].level + 2,
                 actArray[act].bonus + 5);
    }
}

static void fillMissions()
{
    // One mission per chapter, named after its chapter.
    for (int i = 0; i < STORY_CHAPTER_COUNT; ++i)
    {
        seedNode(missionArray[i], chaptersArray[i].name,
                 chaptersArray[i].description, chaptersArray[i].level,
                 chaptersArray[i].bonus);
    }
}

static void fillQuests()
{
    // Two quests per act, spread across the table.
    static const char *questNames[24] = {
        "The Dark Within", "The Warmind", "Eyes Up", "Riptide",
        "A Deadly Trial", "Deep Six", "The Long Goodbye", "Broken Courier",
        "The Signal", "The Hunt", "Frozen Path", "Iron Tomb",
        "Coalition Blues", "Smuggler's Toll", "Mining Rights", "Rim Justice",
        "Brood Mother", "Swarm Cull", "Hive Mind", "The Queen's Fall",
        "Ghost Protocol", "Core Breach", "Machine Mercy", "Override"};

    for (int i = 0; i < 24 && i < STORY_QUEST_COUNT; ++i)
    {
        int act = i / 2;
        seedNode(questArray[i], questNames[i],
                 actArray[act].description,
                 actArray[act].level, actArray[act].bonus);
    }
}

static void fillStoryTables()
{
    fillActs();
    fillChapters();
    fillSeasons();
    fillStages();
    fillMissions();
    fillQuests();
}

/* ---------------- queries ---------------- */

int storyActCount()
{
    return STORY_ACT_COUNT;
}

int storyChapterCount()
{
    return STORY_CHAPTER_COUNT;
}

const storynode *storyAct(int index)
{
    if (index < 0 || index >= STORY_ACT_COUNT)
        return 0;

    return &actArray[index];
}

const storynode *storyChapter(int index)
{
    if (index < 0 || index >= STORY_CHAPTER_COUNT)
        return 0;

    return &chaptersArray[index];
}

// Which act a global chapter number belongs to.
int storyActForChapter(int chapterIndex)
{
    if (chapterIndex < 0 || chapterIndex >= STORY_CHAPTER_COUNT)
        return -1;

    int level = chaptersArray[chapterIndex].level;

    for (int act = STORY_ACT_COUNT - 1; act >= 0; --act)
    {
        if (level >= actArray[act].level)
            return act;
    }

    return 0;
}

/* ---------------- presentation ---------------- */

void showStoryMode()
{
    cout << "\n=========== STORY MODE ===========\n";

    cout << "Act:     " << actArray[currentAct].name
         << " - " << actArray[currentAct].description << "\n";
    cout << "Season:  " << seasonsArray[currentAct].name
         << " - " << seasonsArray[currentAct].description << "\n";
    cout << "Chapter: " << chaptersArray[currentChapter].name
         << " - " << chaptersArray[currentChapter].description << "\n";

    if (currentChapter < STORY_MISSION_COUNT)
    {
        cout << "Mission: " << missionArray[currentChapter].name
             << " - " << missionArray[currentChapter].description << "\n";
    }

    cout << "Quest:   " << questArray[0].name
         << " - " << questArray[0].description << "\n\n";
}

void showActList()
{
    cout << "\n=========== ACTS ===========\n";

    for (int i = 0; i < STORY_ACT_COUNT; ++i)
    {
        cout << "  ";
        if (i == currentAct)
            cout << "> ";
        else
            cout << "  ";

        cout << actArray[i].name << " - " << actArray[i].description << endl;
    }

    cout << endl;
}

void showChapterList(int actIndex)
{
    if (actIndex < 0 || actIndex >= STORY_ACT_COUNT)
        return;

    cout << "\n=========== CHAPTERS ===========\n";

    for (int i = 0; i < STORY_CHAPTER_COUNT; ++i)
    {
        if (storyActForChapter(i) != actIndex)
            continue;

        cout << "  " << chaptersArray[i].name << endl;
        cout << "     " << chaptersArray[i].description << endl;
    }

    cout << endl;
}

// Advance one chapter, rolling into the next act when needed.
int storyAdvanceChapter()
{
    if (currentChapter >= STORY_CHAPTER_COUNT - 1)
        return 0;

    currentChapter++;

    int act = storyActForChapter(currentChapter);

    if (act > currentAct)
    {
        currentAct = act;

        cout << "\n  === " << actArray[currentAct].name << ": "
             << actArray[currentAct].description << " ===\n" << endl;

        audioPlayMusic(MUS_EMPIRE);
    }

    cout << "  " << chaptersArray[currentChapter].name
         << " - " << chaptersArray[currentChapter].description << endl;

    return 1;
}

int storyCurrentAct()
{
    return currentAct;
}

int storyCurrentChapter()
{
    return currentChapter;
}

/* ---------------- entry points ---------------- */

void initializestorymode()
{
    fillStoryTables();

    currentAct = 0;
    currentChapter = 0;

    showStoryMode();
}

/* ---------------- endgame ---------------- */

void initializeendgame()
{
    cout << "\nReplied From 5 Star Admiral Stephen\n"
         << "You are lucky that you got in a escape pod before your ship was blown up\n"
         << "You have failed to beat The Divine Order Imperial StarShips. You have also\n"
         << "failed to save the Milky Way Galaxy from the Divine Order.\n"
         << "What were you thinking going\n"
         << "into battle so weak (*hint make ship stats higher*)!\n"
         << "So now you are in the escape pod and have to wait before you can get home.\n"
         << "Mission Over (Game Over)" << endl;

    audioPlayMusic(MUS_DEFEAT);
    waitForEnter();

    cout << "\nReplied from Captain:\n"
         << "Um...Yeah, I guess, unless\n"
         << "the programmer doesn't feel lazy and\n"
         << "adds more to the game.\n" << endl;
    waitForEnter();

    cout << "     \"Fat chance of that happening, I hardly had the energy to make\n"
         << "the game the way it is now, screw adding more.\"\n"
         << "came a voice from out of nowhere...\n\n"
         << "Also Live long and prosper" << endl;
    waitForEnter();

    cout << "The End" << endl;
    waitForEnter();
}
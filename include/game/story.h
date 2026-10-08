// include/game/story.h -- story mode content and the endgame sequence.
//
// The campaign is 12 acts long and holds 50 chapters.  Act and chapter text
// lives in data tables inside src/game/story.c++; these functions walk them.
#ifndef SBW_GAME_STORY_H
#define SBW_GAME_STORY_H

void initializestorymode();
void initializeendgame();

/* ---------------- campaign queries ---------------- */

// How many acts and chapters the campaign defines (12 and 50).
int storyActCount();
int storyChapterCount();

// Fetch an act or chapter by index.  Returns null when out of range.
const storynode *storyAct(int index);
const storynode *storyChapter(int index);

// Which act a global chapter number belongs to.  Returns -1 if out of range.
int storyActForChapter(int chapterIndex);

/* ---------------- progress ---------------- */

int storyCurrentAct();
int storyCurrentChapter();

// Advance one chapter.  Returns 0 when the campaign is already complete.
int storyAdvanceChapter();

/* ---------------- presentation ---------------- */

// The current act / season / chapter / mission / quest, as a block of text.
void showStoryMode();

// Every act, with the current one marked.
void showActList();

// Every chapter belonging to one act.
void showChapterList(int actIndex);

#endif

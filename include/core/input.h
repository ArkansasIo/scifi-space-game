// include/core/input.h
// Robust input helpers.  Defined in src/core/input.c++.
// These never leave the input stream in a failed state, so one bad entry
// cannot break every later prompt.

#ifndef SBW_CORE_INPUT_H
#define SBW_CORE_INPUT_H

// Read an integer, retrying on bad input.
int readInt(const char *prompt);

// Read a single character, uppercased.  Only the first non-space character of
// the line is used, so typing a whole word cannot leak into the next prompt.
char readChar(const char *prompt);

// Portable replacement for system("PAUSE").
void waitForEnter();

// Random number in the range 1..max (0 if max <= 0).
int randomNumber(int max);

#endif /* SBW_CORE_INPUT_H */

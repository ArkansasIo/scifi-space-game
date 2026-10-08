// src/core/input.c++ -- robust input helpers.

#include "spacebattlerpg.h"
#include <limits>

// Read an integer, retrying on bad input.  On a failed extraction we clear the
// error state and discard the rest of the line, so the stream is left usable
// for every later prompt.
int readInt(const char *prompt)
{
    int value = 0;

    for (;;)
    {
        cout << prompt;
        if (cin >> value)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "That is not a number, please try again.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Read a single character, uppercased.  If the player types a whole word we
// keep only the first non-space character so the rest cannot leak into the
// next prompt.
char readChar(const char *prompt)
{
    string line;

    for (;;)
    {
        cout << prompt;
        cout.flush();

        if (!getline(cin, line))
        {
            // End of input (e.g. piped input ran out, or EOF) - stop cleanly.
            cin.clear();
            return 'Q';
        }

        // Strip CR explicitly: some terminals and all CRLF-redirected input
        // deliver a trailing '\r' that isspace() alone does not catch on
        // every locale, which would otherwise reject a valid line.
        while (!line.empty()
               && (line[line.size() - 1] == '\r'
                   || line[line.size() - 1] == '\n'))
            line.erase(line.size() - 1);

        for (size_t i = 0; i < line.size(); ++i)
        {
            unsigned char c = (unsigned char)line[i];

            if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
                return (char)toupper(c);
        }

        // A genuinely blank line: treat Enter as "no answer" only when the
        // caller can cope with it, otherwise keep asking.  Returning '\0'
        // lets prompts like waitForEnter() advance on a bare Enter.
        if (line.empty())
            return '\0';

        cout << "Please enter a letter.\n";
    }
}

// Portable replacement for system("PAUSE").
void waitForEnter()
{
    cout << "Press Enter to continue...";
    cout.flush();

    string line;
    if (!getline(cin, line))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int randomNumber(int max)
{
    if (max <= 0)
        return 0;
    return (rand() % max) + 1;
}

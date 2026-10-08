// src/ui/display.c++ -- the presentation layer.
//
// Targets a plain 80x24 console (MAXROW / MAXCOL).  No colour codes are
// emitted, so output is identical in a Windows console, a pipe or a log file.

#include "spacebattlerpg.h"
#include "ui/display.h"

/* ---------------- canvas ---------------- */

void clearScreen()
{
    // Plain newlines rather than ANSI, so redirected output stays readable.
    for (int i = 0; i < MAXROW; ++i)
        cout << endl;
}

void scrollClean()
{
    for (int i = 0; i < 2; ++i)
        cout << endl;
}

void drawRule(char fill)
{
    for (int i = 0; i < MAXCOL; ++i)
        cout << fill;
    cout << endl;
}

void drawDoubleRule(char fill)
{
    drawRule(fill);
    drawRule(fill);
}

void drawHeader(const char *title)
{
    int len = title ? (int)strlen(title) : 0;
    if (len > MAXCOL - 8)
        len = MAXCOL - 8;

    cout << "+";
    for (int i = 0; i < len + 6; ++i)
        cout << "-";
    cout << "+" << endl;

    cout << "|   ";
    if (title)
        cout.write(title, len);
    cout << "   |" << endl;

    cout << "+";
    for (int i = 0; i < len + 6; ++i)
        cout << "-";
    cout << "+" << endl;
}

void drawPanel(const char *title, const char *body)
{
    drawHeader(title);

    if (body)
        outWrapped(body);

    cout << endl;
}

void drawCentered(const char *text)
{
    if (!text)
        return;

    int len = (int)strlen(text);
    int pad = (MAXCOL - len) / 2;

    for (int i = 0; i < pad; ++i)
        cout << ' ';

    cout << text << endl;
}

void drawField(const char *label, int value)
{
    cout << "  " << label;

    int len = label ? (int)strlen(label) : 0;
    for (int i = len; i < 22; ++i)
        cout << ' ';

    cout << ": " << value << endl;
}

void drawFieldText(const char *label, const char *value)
{
    cout << "  " << label;

    int len = label ? (int)strlen(label) : 0;
    for (int i = len; i < 22; ++i)
        cout << ' ';

    cout << ": " << (value ? value : "") << endl;
}

/* ---------------- bars ---------------- */

void drawBar(const char *label, int value, int maximum, int width)
{
    if (width < 4)
        width = 4;

    if (maximum <= 0)
        maximum = 1;

    if (value < 0)
        value = 0;

    if (value > maximum)
        value = maximum;

    int filled = (value * width) / maximum;
    int percent = (value * 100) / maximum;

    cout << "  " << label << " [";
    for (int i = 0; i < width; ++i)
        cout << (i < filled ? '#' : '-');
    cout << "] " << percent << "%" << endl;
}

void drawHealthBar(int current, int maximum)
{
    drawBar("Hull ", current, maximum, 20);
}

void drawShieldBar(int current, int maximum)
{
    drawBar("SIF  ", current, maximum, 20);
}

void drawPowerBar(int current, int maximum)
{
    drawBar("Power", current, maximum, 20);
}

/* ---------------- text output ---------------- */

void out(const char *text)
{
    cout << "  " << (text ? text : "") << endl;
}

void outNumber(const char *label, int value)
{
    cout << "  " << (label ? label : "") << ": " << value << endl;
}

void outWrapped(const char *text)
{
    if (!text)
        return;

    const int width = MAXCOL - 4;
    int column = 0;

    cout << "  ";

    for (const char *p = text; *p; ++p)
    {
        if (*p == '\n')
        {
            cout << endl
                 << "  ";
            column = 0;
            continue;
        }

        cout << *p;
        column++;

        if (column >= width)
        {
            // Wrap at the last space if there is one nearby.
            cout << endl
                 << "  ";
            column = 0;
        }
    }

    cout << endl;
}

/* ---------------- text input ---------------- */

int askInt(const char *prompt, int minimum, int maximum)
{
    for (;;)
    {
        int value = readInt(prompt);

        if (value >= minimum && value <= maximum)
            return value;

        cout << "  Please enter a value between " << minimum
             << " and " << maximum << "." << endl;
    }
}

int askYesNo(const char *prompt)
{
    for (;;)
    {
        // readChar() consumes the whole line, so it cannot desync the stream
        // after a preceding readInt()/getline().
        char answer = readChar(prompt);

        if (answer == 'Y')
            return 1;
        if (answer == 'N')
            return 0;

        cout << "  Please answer Y or N." << endl;
    }
}

const char *askText(const char *prompt, char buffer[], int bufferSize)
{
    if (!buffer || bufferSize <= 1)
        return "";

    cout << prompt;
    cout.flush();

    string line;

    // Use getline on a std::string so the prompt/stream stay in sync with
    // readInt() and readChar(), both of which consume whole lines.
    if (!getline(cin, line))
    {
        cin.clear();
        buffer[0] = '\0';
        return buffer;
    }

    int copy = (int)line.size();

    if (copy > bufferSize - 1)
        copy = bufferSize - 1;

    for (int i = 0; i < copy; ++i)
        buffer[i] = line[i];

    buffer[copy] = '\0';

    return buffer;
}

void pressEnterToContinue()
{
    waitForEnter();
}

/* ---------------- menus ---------------- */

int showMenu(const char *title, const char *options[], int optionCount,
             int allowCancel)
{
    if (title)
        drawHeader(title);

    for (int i = 0; i < optionCount; ++i)
        cout << "   " << (i + 1) << ") " << options[i] << endl;

    if (allowCancel)
        cout << "   0) Cancel" << endl;

    cout << endl;

    int maximum = optionCount;
    int minimum = allowCancel ? 0 : 1;

    int choice = askInt("Choice: ", minimum, maximum);

    if (choice == 0)
        return -1;

    return choice - 1;
}

char showKeyMenu(const char *title, const char *keys,
                 const char *descriptions[], int count)
{
    if (title)
        drawHeader(title);

    for (int i = 0; i < count; ++i)
        cout << "   " << keys[i] << " - " << descriptions[i] << endl;

    cout << endl;

    for (;;)
    {
        char choice = readChar("Choice: ");

        for (int i = 0; i < count; ++i)
        {
            if (choice == keys[i])
                return choice;
        }

        cout << "  Not a valid key." << endl;
    }
}

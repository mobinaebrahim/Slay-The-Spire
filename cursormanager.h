#ifndef CURSORMANAGER_H
#define CURSORMANAGER_H

#include <QString>

class CursorManager
{
public:
    static void applyCursor(int cursorIndex);
    static void applySavedCursor();

private:
    static QString pathForIndex(int index);
};

#endif // CURSORMANAGER_H
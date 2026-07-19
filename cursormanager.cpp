#include "cursormanager.h"
#include <QApplication>
#include <QCursor>
#include <QPixmap>
#include <QSettings>

QString CursorManager::pathForIndex(int index)
{
    switch (index) {
    case 0: return ":/assets/mainmenu/cursor1.png";
    case 1: return ":/assets/mainmenu/cursor2.png";
    case 2: return ":/assets/mainmenu/cursor3.png";
    default: return ":/assets/mainmenu/cursor1.png";
    }
}


#endif
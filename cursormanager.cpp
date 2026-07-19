#include "cursormanager.h"

QString CursorManager::pathForIndex(int index)
{
    switch (index) {
    case 0: return ":/assets/mainmenu/cursor1.png";
    case 1: return ":/assets/mainmenu/cursor2.png";
    case 2: return ":/assets/mainmenu/cursor3.png";
    default: return ":/assets/mainmenu/cursor1.png";
    }
}

void CursorManager::applyCursor(int cursorIndex)
{
    QPixmap pix(pathForIndex(cursorIndex));
    if (pix.isNull())
        return;

    QPixmap scaled = pix.scaled(45, 45, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QCursor customCursor(scaled, 0, 0);
    qApp->restoreOverrideCursor();
    qApp->setOverrideCursor(customCursor);

    QSettings settings;
    settings.setValue("appearance/cursorIndex", cursorIndex);
}

void CursorManager::applySavedCursor()
{
    QSettings settings;
    int savedIndex = settings.value("appearance/cursorIndex", 0).toInt();
    applyCursor(savedIndex);
}
#include "display-manager.h"

#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

DisplayManager::DisplayManager() {
    m_screens = QGuiApplication::screens();
}

bool DisplayManager::hasMultipleDisplays() const {
    return m_screens.size() > 1;
}

void DisplayManager::showOnDisplay(QWidget *window, int displayIndex) {
    if (displayIndex < 0 || displayIndex > m_screens.size()) {
        qDebug() << "Invalid display index. Showing on the primary display.";

        window->show();
        return;
    }

    QScreen* screen = m_screens.at(displayIndex);
    QRect screenGeometry = screen->availableGeometry();

    window->setGeometry(screenGeometry);
    window->setWindowFlags(Qt::FramelessWindowHint);
    window->showFullScreen();
}

void DisplayManager::showOnPrimaryDisplay(QWidget *window) {
    showOnDisplay(window, 0);
}

void DisplayManager::showOnSecondaryDisplay(QWidget *window) {
    if (hasMultipleDisplays()) {
        showOnDisplay(window, 1);
    }  else {
        qDebug() << "Only one display is connected. Showing on the primary display.";
        showOnPrimaryDisplay(window);
    }
}

QSize DisplayManager::secondaryDisplaySize() const {
    return m_screens.at(1)->size();
}

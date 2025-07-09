#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <QList>
#include <QScreen>
#include <QWidget>

class DisplayManager
{
public:
    DisplayManager();

public:
    bool hasMultipleDisplays() const;
    void showOnDisplay(QWidget*, int);
    void showOnPrimaryDisplay(QWidget*);
    void showOnSecondaryDisplay(QWidget*);

    QSize secondaryDisplaySize() const;
    QSize primaryDisplaySize() const;

private:
    QList<QScreen*> m_screens;
};

#endif // DISPLAY_MANAGER_H

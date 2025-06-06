#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include "windows/main_window/mainwindow.h"

#include <QObject>
#include <QMainWindow>
#include <QStackedWidget>

class MainController : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainController(QWidget *parent = nullptr);

signals:

private:
    QStackedWidget *m_stackedWidget;
    MainWindow *m_mainWindow;
};

#endif // MAINCONTROLLER_H

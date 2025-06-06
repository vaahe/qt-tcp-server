#include "main-controller.h"

MainController::MainController(QWidget *parent) :
    QMainWindow(parent),
    m_stackedWidget(new QStackedWidget(this)),
    m_mainWindow(new MainWindow()) {
    m_stackedWidget->addWidget(m_mainWindow);

    setCentralWidget(m_stackedWidget);
    m_stackedWidget->setCurrentIndex(0);
}

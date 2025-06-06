#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelectionModel>

#include "database/database-manager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_userModel(nullptr)
    , m_trainingModel(nullptr)
{
    ui->setupUi(this);

    auto &db = DatabaseManager::getInstance();

    auto *splitter = new QSplitter(this);
    setCentralWidget(splitter);

    m_userModel = new QSqlTableModel(this, db.m_db);
    m_userModel->setTable("users");
    m_userModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_userModel->select();

    auto *userView = new QTableView(splitter);
    userView->setModel(m_userModel);
    userView->setSelectionBehavior(QAbstractItemView::SelectRows);
    userView->setSelectionMode(QAbstractItemView::SingleSelection);
    userView->horizontalHeader()->setStretchLastSection(true);

    m_trainingModel = new QSqlTableModel(this, db.m_db);
    m_trainingModel->setTable("trainings");
    m_trainingModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_trainingModel->select();

    auto *trainView = new QTableView(splitter);
    trainView->setModel(m_trainingModel);
    trainView->setSelectionBehavior(QAbstractItemView::SelectRows);
    trainView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    trainView->horizontalHeader()->setStretchLastSection(true);

    connect(userView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::onUserSelectionChanged);

    if (m_userModel->rowCount() > 0) {
        QModelIndex first = m_userModel->index(0, 0);
        userView->selectionModel()->setCurrentIndex(first, QItemSelectionModel::SelectCurrent);
    }

    m_debugWidget.show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onUserSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    int userId = m_userModel->data(m_userModel->index(current.row(), 0)).toInt();

    m_trainingModel->setFilter(QString("user_id = %1").arg(userId));
    m_trainingModel->select();
}

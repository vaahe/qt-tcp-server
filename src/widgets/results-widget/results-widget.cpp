#include "results-widget.h"
#include "database/database-manager.h"
#include <QSqlTableModel>
#include <QSplitter>
#include <QTableView>

#include <QDebug>
#include <QSqlError>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QHeaderView>

ResultsTableWidget::ResultsTableWidget(const QString &tableName, QWidget *parent)
    : QWidget(parent)
{
    m_model = new QSqlTableModel(this, QSqlDatabase::database());
    m_model->setTable(tableName);
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!m_model->select()) {
        qWarning() << "[ResultsTableWidget] Failed to load table" << tableName
                   << ":" << m_model->lastError().text();
    }

    // Set up the view
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->horizontalHeader()->setStretchLastSection(true);

    // Refresh button
    m_refreshButton = new QPushButton(tr("Refresh"), this);
    connect(m_refreshButton, &QPushButton::clicked, this, &ResultsTableWidget::refresh);

    // Layout
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_tableView);
    layout->addWidget(m_refreshButton);
    setLayout(layout);
}

ResultsTableWidget::~ResultsTableWidget() = default;

void ResultsTableWidget::refresh()
{
    m_model->select();
}

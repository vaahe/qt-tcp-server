#ifndef RESULTS_TABLE_WIDGET_H
#define RESULTS_TABLE_WIDGET_H

#include <QWidget>
#include <QSqlTableModel>

class QTableView;
class QPushButton;

/**
 * @brief Generic widget to display a database table in a read-only QTableView.
 */
class ResultsTableWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct a widget for viewing the given table.
     * @param tableName Name of the table in the SQLite database.
     * @param parent    Optional parent widget.
     */
    explicit ResultsTableWidget(const QString &tableName, QWidget *parent = nullptr);
    ~ResultsTableWidget() override;

public slots:
    /**
     * @brief Refreshes the view by re-reading from the database.
     */
    void refresh();

private:
    QSqlTableModel *m_model;
    QTableView    *m_tableView;
    QPushButton   *m_refreshButton;
};

#endif // RESULTS_TABLE_WIDGET_H

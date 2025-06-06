#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "widgets/debug-widget/debug-widget.h"
#include "widgets/results-widget/results-widget.h"

#include <QMainWindow>
#include <QSqlTableModel>
#include <QSplitter>
#include <QTableView>
#include <QItemSelectionModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onUserSelectionChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    Ui::MainWindow *ui;
    QSqlTableModel *m_userModel;
    QSqlTableModel *m_trainingModel;
    DebugWidget &m_debugWidget = DebugWidget::getInstance();
};
#endif // MAINWINDOW_H

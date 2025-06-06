#ifndef DEBUG_WIDGET_H
#define DEBUG_WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QLayoutItem>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>

#include "clients/client.h"

class DebugWidget : public QWidget {
    Q_OBJECT

public:
    static DebugWidget& getInstance();
    void setupUI();

public slots:
    void onSendSignal(QTcpSocket* client, const QByteArray& data);
    void onReceiveSignal(QTcpSocket* client, const QByteArray& data);
    void onDataSignalTrigger(QTcpSocket* client, const QByteArray& data, const QColor& color, const QString action);

signals:
    void sendSignal(QTcpSocket* client, const QByteArray& data);
    void receiveSignal(QTcpSocket* client, const QByteArray& data);

private:
    DebugWidget(const DebugWidget&) = delete;
    void operator=(const DebugWidget&) = delete;

    QScrollArea* m_scrollArea;
    QWidget* m_container;
    QVBoxLayout* m_signalsLayout;

    QLineEdit*     m_input;
    QPushButton*   m_btn1;
    QPushButton*   m_btn2;
    QPushButton*   m_btn3;
    QPushButton*   m_btn4;

    Client *m_client1;
    Client *m_client2;
    Client *m_client3;
    Client *m_client4;

protected:
    explicit DebugWidget(QWidget* parent = nullptr);

    ~DebugWidget() override = default;
};

#endif // DEBU_GWIDGET_H

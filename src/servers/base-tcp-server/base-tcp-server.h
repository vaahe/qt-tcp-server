#ifndef BASESERVER_H
#define BASESERVER_H

#include <QList>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "widgets/debug-widget/debug-widget.h"

class BaseTcpServer : public QObject
{
    Q_OBJECT

public:
    BaseTcpServer(quint16 port, QObject *parent = nullptr);
    ~BaseTcpServer() override;

public:
    bool connect();
    void disconnect();
    void send(QTcpSocket* client, const QByteArray& data);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();
    void onSocketError(QAbstractSocket::SocketError socketError);

    virtual void onDataReceived(QTcpSocket* client, const QByteArray& data) = 0;

signals:
    void newConnection(QTcpSocket* client);
    void dataReceived(QTcpSocket* client, const QByteArray& data);
    void dataSent(QTcpSocket* client, const QByteArray& data);
    void clientDisconnected(QTcpSocket* client);
    void errorOccurred(QTcpSocket* client, QAbstractSocket::SocketError socketError);

private:
    QScopedPointer<QTcpServer> m_server;
    QList<QTcpSocket*> m_clients;

    DebugWidget &m_debugWidget = DebugWidget::getInstance();

protected:
    const QList<QTcpSocket*>& clients() const { return m_clients; }

public:
    quint16 m_port;
};

#endif // BASESERVER_H

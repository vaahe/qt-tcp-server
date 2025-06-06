#include "base-tcp-server.h"

BaseTcpServer::BaseTcpServer(quint16 port, QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_port(port) {
    QObject::connect(m_server.data(), &QTcpServer::newConnection, this, &BaseTcpServer::onNewConnection);
    QObject::connect(this, &BaseTcpServer::dataReceived, &m_debugWidget, &DebugWidget::onReceiveSignal);
    QObject::connect(this, &BaseTcpServer::dataSent, &m_debugWidget, &DebugWidget::onSendSignal);
}

BaseTcpServer::~BaseTcpServer() {
    disconnect();
}

bool BaseTcpServer::connect() {
    if (!m_server->listen(QHostAddress::Any, m_port)) {
        qWarning() << "BaseTcpServer: listen failed on port" << m_port << ":" << m_server->errorString();
        return false;
    }

    qDebug() << "BaseTcpServer: listening on port" << m_port;
    return true;
}

void BaseTcpServer::disconnect() {
    if (m_server->isListening()) {
        m_server->close();
    }

    for (QTcpSocket *client : std::as_const(m_clients)) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->disconnectFromHost();
        }
        client->deleteLater();
    }

    m_clients.clear();
}

void BaseTcpServer::send(QTcpSocket *client, const QByteArray &data) {
    if (!client) {
        return;
    }

    if (client->state() == QAbstractSocket::ConnectedState) {
        client->write(data);
        emit dataSent(client, data);
    } else {
        qWarning() << "BaseTcpServer: cannot send data to client";
    }
}

void BaseTcpServer::onNewConnection() {
    while (m_server->hasPendingConnections()) {
        QTcpSocket *client = m_server->nextPendingConnection();
        m_clients.append(client);

        QObject::connect(client, &QTcpSocket::readyRead, this, &BaseTcpServer::onReadyRead);
        QObject::connect(client, &QTcpSocket::disconnected, this, &BaseTcpServer::onClientDisconnected);
        QObject::connect(client, &QTcpSocket::errorOccurred, this, &BaseTcpServer::onSocketError);

        emit newConnection(client);
    }
}

void BaseTcpServer::onReadyRead() {
    QTcpSocket* client =  qobject_cast<QTcpSocket*>(sender());
    if (!client) {
        return;
    }

    QByteArray data = client->readAll();
    emit dataReceived(client, data);
}

void BaseTcpServer::onClientDisconnected() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) {
        return;
    }

    m_clients.removeAll(client);
    emit clientDisconnected(client);
    client->deleteLater();
}

void BaseTcpServer::onSocketError(QAbstractSocket::SocketError socketError) {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    emit errorOccurred(client, socketError);
}

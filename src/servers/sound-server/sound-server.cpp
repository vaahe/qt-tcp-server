#include "sound-server.h"
#include <QDebug>

SoundServer::SoundServer(quint16 port, QObject* parent) : BaseTcpServer(port, parent) {}

SoundServer::~SoundServer() {}

void SoundServer::onDataReceived(QTcpSocket* client, const QByteArray& data) {
    const QString cmd = QString::fromUtf8(data).trimmed();

    qDebug() << "[CommandsServer] Received from" << client->peerAddress().toString() << ":" << cmd;

    send(client, data);
}

void SoundServer::sendToAllClients(const QByteArray& data) {
    for (QTcpSocket* client : clients()) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}

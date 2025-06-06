#ifndef SOUND_SERVER_H
#define SOUND_SERVER_H

#include "servers/base-tcp-server/base-tcp-server.h"

class SoundServer : public BaseTcpServer
{
    Q_OBJECT

public:
    explicit SoundServer(quint16 port, QObject* parent = nullptr) : BaseTcpServer(port, parent) {
        QObject::connect(this, &BaseTcpServer::dataReceived, this, &SoundServer::onDataReceived);

        if (!this->connect()) {
            qFatal("EchoServer failed to listen on port %u", port);
        }
    }

public:
    void onDataReceived(QTcpSocket* client, const QByteArray& data) {
        send(client, data); // send data back to client
    }
};

#endif // SOUND_SERVER_H

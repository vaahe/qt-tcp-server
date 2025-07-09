#ifndef SOUND_SERVER_H
#define SOUND_SERVER_H

#include "servers/base-tcp-server/base-tcp-server.h"

class SoundServer : public BaseTcpServer
{
    Q_OBJECT

private:
    explicit SoundServer(quint16 port = 3001, QObject* parent = nullptr);
    ~SoundServer() override;

public:
    static SoundServer& getInstance() {
        static SoundServer instance;
        return instance;
    }

    void sendToAllClients(const QByteArray& data);

protected:
    void onDataReceived(QTcpSocket* client, const QByteArray& data) override;
};

#endif // SOUND_SERVER_H

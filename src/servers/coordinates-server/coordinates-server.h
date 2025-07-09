#ifndef CAMERA_SERVER_H
#define CAMERA_SERVER_H

#include "calibration/camera-calibration.h"
#include "servers/base-tcp-server/base-tcp-server.h"

#include <QThread>

class CoordinatesServer : public BaseTcpServer
{
    Q_OBJECT

private:
    explicit CoordinatesServer(quint16 port = 3000, QObject* parent = nullptr);
    ~CoordinatesServer() override;

public:
    static CoordinatesServer& getInstance() {
        static CoordinatesServer instance;
        return instance;
    }

    void sendToAllClients(const QByteArray& data);

protected:
    void onDataReceived(QTcpSocket* client, const QByteArray& data) override;
};

#endif // CAMERA_SERVER_H

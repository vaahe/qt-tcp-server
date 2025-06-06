#ifndef CAMERA_SERVER_H
#define CAMERA_SERVER_H

#include "calibration/camera-calibration.h"
#include "servers/base-tcp-server/base-tcp-server.h"

#include <QThread>

class CoordinatesServer : public BaseTcpServer
{
    Q_OBJECT

public:
    explicit CoordinatesServer(quint16 port, QObject* parent = nullptr);
    ~CoordinatesServer() override;

protected:
    void onDataReceived(QTcpSocket* client, const QByteArray& data) override;
};

#endif // CAMERA_SERVER_H

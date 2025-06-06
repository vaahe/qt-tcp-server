#ifndef SENSOR_SERVER_H
#define SENSOR_SERVER_H

#include "serialport/serial-port-manager.h"
#include "servers/base-tcp-server/base-tcp-server.h"

class SensorServer : public BaseTcpServer
{
    Q_OBJECT

public:
    explicit SensorServer(quint16 port, QObject* parent = nullptr);
    ~SensorServer() override = default;

protected slots:
    // Handle data arriving from TCP clients
    void onDataReceived(QTcpSocket* client, const QByteArray& data) override;

private slots:
    // Handle data arriving from the serial port
    void onSerialData(const QByteArray& data);

private:
    SerialPortManager* m_serialManager;
    QList<QTcpSocket*> m_clients;
};


#endif // SENSOR_SERVER_H

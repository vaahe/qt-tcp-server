#include "sensor-server.h"
#include <QDebug>
#include <QSerialPortInfo>

SensorServer::SensorServer(quint16 port, QObject* parent)
    : BaseTcpServer(port, parent), m_serialManager(new SerialPortManager())
{
    QObject::connect(this, &BaseTcpServer::newConnection, this, [this](QTcpSocket* client) {
        m_clients.append(client);

        QObject::connect(client, &QTcpSocket::disconnected, this, [this, client]() {
            m_clients.removeAll(client);
        });
    });

    // Forward incoming TCP data to serial port
    QObject::connect(this, &BaseTcpServer::dataReceived, this, &SensorServer::onDataReceived);

    // Broadcast serial data back to TCP clients
    QObject::connect(m_serialManager, &SerialPortManager::dataSent, this, &SensorServer::onSerialData);

    // Start listening
    if (!BaseTcpServer::connect()) {
        qFatal("SensorServer failed to listen on port %u", port);
    }
}

void SensorServer::onDataReceived(QTcpSocket* client, const QByteArray& data)
{
    qDebug() << "[SensorServer] TCP->Serial:" << data;
    m_serialManager->write(data);
}

void SensorServer::onSerialData(const QByteArray& data)
{
    qDebug() << "[SensorServer] Serial->TCP:" << data;
    // Send to all connected TCP clients
    for (QTcpSocket* client : m_clients) {
        send(client, data);
    }
}

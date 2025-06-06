#include "coordinates-server.h"
#include <QDebug>

CoordinatesServer::CoordinatesServer(quint16 port, QObject* parent)
    : BaseTcpServer(port, parent)
{

}

CoordinatesServer::~CoordinatesServer()
{

}

void CoordinatesServer::onDataReceived(QTcpSocket* client, const QByteArray& data)
{
    const QString cmd = QString::fromUtf8(data).trimmed();

}

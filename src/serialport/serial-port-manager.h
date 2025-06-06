#ifndef SERIAL_PORT_MANAGER_H
#define SERIAL_PORT_MANAGER_H

#include "clients/client.h"
#include "utils/json-parser/json-parser.h"
#include "utils/globals-manager/globals-manager.h"

#include <QFile>
#include <QDebug>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QSerialPort>
#include <QJsonDocument>
#include <QTextStream>
#include <QSerialPortInfo>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    SerialPortManager();
    ~SerialPortManager();

public slots:
    void disconnect();
    void read();
    void write(const QByteArray&);
    void error(QSerialPort::SerialPortError);
    void writeToJson(const QJsonObject &jsonObject);
    void initializeComPort();

    void sendInitialData();

signals:
    void isConnected();
    void isDisconnected();
    void isPending();
    void errorOccurred();
    void pressingStopped();
    void dataSent(const QByteArray& data);
    void shotDataSent(const QByteArray& data);

private:
    QByteArray m_buffer;
    QSerialPort *m_serialPort;
    GlobalsManager &m_globalsManager = GlobalsManager::getInstance();
};

#endif // SERIAL_PORT_MANAGER_H

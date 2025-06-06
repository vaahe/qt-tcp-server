#include "serial-port-manager.h"

SerialPortManager::SerialPortManager() : m_serialPort(new QSerialPort)
{
    qDebug() << "SerialPort instance is created";
    sendInitialData();

    QObject::connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPortManager::error);
    QObject::connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortManager::read);
}

SerialPortManager::~SerialPortManager() {
    qDebug() << "SerialPort instance is destroyed";
}

void SerialPortManager::disconnect() {
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        emit isDisconnected();
        qDebug() << "Disconnected from: " << m_serialPort->portName();
    }
}

void SerialPortManager::read() {
    QByteArray data = m_serialPort->readAll();
    char endMarker = '\n';

    m_buffer.append(data);

    qDebug() << "line 63:" << m_buffer;

    if (m_buffer.contains("x\n")) {
        emit shotDataSent("x\n");
        int shotIndex = m_buffer.indexOf("x\n");

        m_buffer.remove(shotIndex, 6);
    } else {
        while(m_buffer.contains(endMarker)) {
            int dataIndex = m_buffer.indexOf(endMarker);
            QByteArray sensorData = m_buffer.left(dataIndex);
            sensorData.append(',');

            m_buffer.remove(0, dataIndex + 1);
            emit dataSent(sensorData);
        }
    }
}


void SerialPortManager::write(const QByteArray &data) {
    try {
        if (!m_serialPort->isOpen()) {
            qDebug() << "SerialPort is not connected";
            return;
        }

        QByteArray dataWithNLCR = data;
        dataWithNLCR.append("\r\n");

        m_serialPort->write(dataWithNLCR);

        QString sentText = QString::fromUtf8(dataWithNLCR);
        qDebug() << "Sent text:" << sentText;
    } catch (const std::exception &e) {
        qDebug() << "Exception while writing to serial port:" << e.what();
    } catch (...) {
        qDebug() << "Unknown exception caught during serial write";
    }
}

void SerialPortManager::error(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        disconnect();
        qDebug() << "Critical error:" << m_serialPort->errorString();
    }
}

void SerialPortManager::sendInitialData() {
    emit dataSent("0");
}

void SerialPortManager::writeToJson(const QJsonObject &jsonObject) {
    QString err;

    const QString filePath = "C:\\Shoot\\sensor.json";
    if (JsonParser::writeJsonToFile(filePath, jsonObject, err)) {
        qDebug() << "JSON created successfully";
    } else {
        qWarning() << "Failed to create JSON:" << err;
    }
}

void SerialPortManager::initializeComPort()
{
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        const QString name = info.portName();

        m_serialPort->setPortName(name);
        m_serialPort->setBaudRate(QSerialPort::Baud57600);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setStopBits(QSerialPort::OneStop);
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

        if (!m_serialPort->open(QIODevice::ReadWrite)) {
            continue;
        }

        m_serialPort->clear(QSerialPort::AllDirections);

        QByteArray probe = "999\n";
        m_serialPort->write(probe);
        if (!m_serialPort->waitForBytesWritten(100)) {
            qDebug() << "[SerialProbe]" << name << "failed to write probe";
            m_serialPort->close();
            continue;
        }

        if (m_serialPort->waitForReadyRead(200)) {
            QByteArray response = m_serialPort->readAll().trimmed();
            qDebug() << "[SerialProbe]" << name << "->" << response;
            if (response == "999") {
                qDebug() << "[SerialProbe] Found matching port:" << name;
                m_globalsManager.setComPort(name);

                return;
            }
        } else {
            qDebug() << "[SerialProbe]" << name << "no response";
        }
    }

    qWarning() << "[SerialProbe] No port echoed '999'; using default";
}

#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>

class Client : public QObject {
    Q_OBJECT

public:
    Client(const QString& host, quint16 port, QObject* parent = nullptr)
        : QObject(parent), m_socket(new QTcpSocket(this))
    {
        connect(m_socket, &QTcpSocket::connected, this, &Client::onConnected);
        connect(m_socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
        connect(m_socket, &QTcpSocket::errorOccurred, this, &Client::onError);

        m_socket->connectToHost(host, port);
    }

signals:
    void dataReceived(const QByteArray& frame);
    void errorOccurred(const QString& what);

public slots:
    void onConnected() {
        QString dnsName = m_socket->peerName();
        QString ip = m_socket->peerAddress().toString();
        quint16 port = m_socket->peerPort();

        qDebug() << "Connected to server:" << (!dnsName.isEmpty() ? dnsName : ip) << "on port" << port;

        // m_socket->write("start calibration");
    }

    void send(const QByteArray& data) {
        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            m_socket->write(data);
        } else {
            qWarning() << "Client::send() called but socket not connected";
        }
    }

    void onReadyRead() {
        QByteArray data = m_socket->readAll();
        emit dataReceived(data);
    }

    void onError(QAbstractSocket::SocketError) {
        emit errorOccurred(m_socket->errorString());
    }

private:
    QTcpSocket* m_socket;
};

#endif // CLIENT_H

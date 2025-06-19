#ifndef COMMANDS_SERVER_H
#define COMMANDS_SERVER_H

#include "database/database-manager.h"
#include "processing/frame-processor.h"
#include "calibration/camera-calibration.h"
#include "clients/http-client/http-client.h"
#include "servers/base-tcp-server/base-tcp-server.h"

#include <QFile>
#include <QThread>
#include <functional>
#include <QApplication>

class CommandsServer : public BaseTcpServer
{
    Q_OBJECT

public:
    explicit CommandsServer(quint16 port, QObject* parent = nullptr);
    ~CommandsServer() override;

protected:
    void onDataReceived(QTcpSocket* client, const QByteArray& data) override;
    void startProcessing(QTcpSocket* client);
    void startCalibration(QTcpSocket* client);
    void printResults(const QString filters);
    void signUp();
    void createMilitaryUnit();
    void closeProcess();
    void createDivision();
    void createSubdivision();

private:
    DatabaseManager &m_dbManager;
    QHash<QString, std::function<void(QTcpSocket*)>> m_commands;
};

#endif // COMMANDS_SERVER_H

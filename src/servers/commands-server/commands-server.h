#ifndef COMMANDS_SERVER_H
#define COMMANDS_SERVER_H

#include "database/database-manager.h"
#include "processing/frame-processor.h"
#include "calibration/camera-calibration.h"
#include "clients/http-client/http-client.h"
#include "utils/printer-manager/printermanager.h"

#include "servers/sound-server/sound-server.h"
#include "servers/base-tcp-server/base-tcp-server.h"
#include "servers/coordinates-server/coordinates-server.h"

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
    void signIn();
    void createMilitaryUnit();
    void closeProcess();
    void createDivision();
    void createSubdivision();
    void saveResult();
    void printData();

signals:
    void stopProcessing();

private:
    DatabaseManager &m_dbManager;
    QHash<QString, std::function<void(QTcpSocket*)>> m_commands;

    SoundServer &m_soundServer = SoundServer::getInstance();
    CoordinatesServer &m_coordinatesServer = CoordinatesServer::getInstance();
};

#endif // COMMANDS_SERVER_H

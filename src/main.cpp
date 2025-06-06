#include "database/database-manager.h"
#include "controllers/main-controller.h"

#include <QLocale>
#include <QTranslator>
#include <QApplication>

#include "clients/client.h"
#include "servers/sound-server/sound-server.h"
#include "servers/sensor-server/sensor-server.h"
#include "servers/commands-server/commands-server.h"
#include "servers/coordinates-server/coordinates-server.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DatabaseManager &dbManager = DatabaseManager::getInstance();
    dbManager.connect();

    if (dbManager.isConnected()) {
        dbManager.seedMockData();
        dbManager.exportDataToJson("C:\\Shoot\\data.json");
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qt-tcp-server_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    MainController mainController;
    mainController.setWindowState(Qt::WindowMaximized);
    mainController.show();

    CoordinatesServer coordinatesServer(3000);
    SoundServer soundServer(3001);
    CommandsServer commandsServer(3002);
    SensorServer sensorServer(3003);

    // Client* coordinatesClient = new Client("127.0.0.1", coordinatesServer.m_port, &app);
    // Client* soundClient = new Client("127.0.0.1", soundServer.m_port, &app);
    Client* commandsClient = new Client("127.0.0.1", commandsServer.m_port, &app);
    // Client* sensorClient = new Client("127.0.0.1", sensorServer.m_port, &app);

    // QObject::connect(coordinatesClient, &Client::dataReceived, &app,[](const QByteArray&){
    //     qDebug() << "Received from CoordinatesServer";
    // });

    // QObject::connect(soundClient, &Client::dataReceived, &app, [](const QByteArray&){
    //     qDebug() << "Received from SoundServer";
    // });

    QObject::connect(commandsClient, &Client::dataReceived, &app, [](const QByteArray&){
        qDebug() << "Received from CommandsServer";
    });

    // QObject::connect(sensorClient, &Client::dataReceived, &app, [](const QByteArray&){
    //     qDebug() << "Received from SensorServer";
    // });

    return app.exec();
}

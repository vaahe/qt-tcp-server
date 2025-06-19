#include "commands-server.h"
#include <QDebug>

CommandsServer::CommandsServer(quint16 port, QObject* parent) :
    BaseTcpServer(port, parent), m_dbManager(DatabaseManager::getInstance())
{
    m_commands = {
                  {"sign_up",            [this](QTcpSocket*)   { signUp(); }},
                  {"close_process",      [this](QTcpSocket*)   { closeProcess(); }},
                  {"create_division",    [this](QTcpSocket*)   { createDivision(); }},
                  {"create_subdivision", [this](QTcpSocket*)   { createSubdivision(); }},
                  {"create_mil_unit",    [this](QTcpSocket*)   { createMilitaryUnit(); }},
                  {"start_processing",   [this](QTcpSocket* c) { startProcessing(c); }},
                  {"start_calibration",  [this](QTcpSocket* c) { startCalibration(c); }},
                };

    QObject::connect(this, &CommandsServer::dataReceived, this, &CommandsServer::onDataReceived);

    if (!connect()) {
        qFatal("CommandsServer failed to listen on port %u", port);
    }
}

CommandsServer::~CommandsServer() {}

void CommandsServer::onDataReceived(QTcpSocket* client, const QByteArray& data) {
    const QString cmd = QString::fromUtf8(data).trimmed().toLower();
    qDebug() << "[CommandsServer] Received from" << client->peerAddress().toString() << ":" << cmd;

    auto it = m_commands.find(cmd);
    if (it != m_commands.end()) {
        it.value()(client);
    } else {
        send(client, "Unknown command\n");
    }
}

void CommandsServer::startCalibration(QTcpSocket* client) {
    static bool moved = false;

    CameraCalibration* calibrator = new CameraCalibration();
    QThread* calibrationThread = new QThread(this);

    if (!moved) {
        calibrator->moveToThread(calibrationThread);
        moved = true;
    }

    if (!calibrationThread->isRunning()) {
        calibrationThread->start();
    }

    QObject::connect(calibrator, &CameraCalibration::calibrationFinished, calibrationThread, &QThread::quit);
    QObject::connect(calibrator, &CameraCalibration::calibrationSucceeded, this, [this, client]() { send(client, "finished\n"); });
    QObject::connect(calibrator, &CameraCalibration::calibrationFailed, this, [this, client]() { send(client, "calibrationFailed\n"); });
    QObject::connect(calibrator, &CameraCalibration::calibrationStarted, this, [this, client]() { send(client, "calibrationStarted\n"); });
    QObject::connect(calibrationThread, &QThread::started,calibrator, &CameraCalibration::startCalibration);
    QObject::connect(calibrationThread, &QThread::finished, calibrator, &QObject::deleteLater);
}

void CommandsServer::startProcessing(QTcpSocket* client) {
    static bool moved = false;

    FrameProcessor* frameProcessor = new FrameProcessor();
    QThread* processingThread = new QThread(this);

    if (!moved) {
        frameProcessor->moveToThread(processingThread);
        moved = true;
    }

    if (!processingThread->isRunning()) {
        processingThread->start();
    }

    QObject::connect(frameProcessor, &FrameProcessor::processingStarted, this, [this, client]() { send(client, "processingStarted\n"); });
    QObject::connect(frameProcessor, &FrameProcessor::processingFinished, processingThread, &QThread::quit);
    QObject::connect(processingThread, &QThread::started, frameProcessor, &FrameProcessor::startProcessing);
    QObject::connect(processingThread, &QThread::finished, frameProcessor, &QObject::deleteLater);
}

void CommandsServer::closeProcess() {
    QApplication::quit();
}

void CommandsServer::signUp() {
    QFile file("C:/Ak-74Utilities/Data/SignUpData.json");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open JSON file:" << file.errorString();
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON format:" << parseError.errorString();
        return;
    }

    QJsonObject obj = doc.object();

    QString fullName = obj.value("full_name").toString();
    QString rank = obj.value("rank").toString();
    QString position = obj.value("position").toString();
    QString subdivisionId = obj.value("subdivision_id").toString();

    m_dbManager.signUp(fullName, rank, position, subdivisionId);
    m_dbManager.exportDataToJson();
}

void CommandsServer::createMilitaryUnit() {
    QFile file("C:/Ak-74Utilities/Data/CreateMilUnitData.json");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open JSON file:" << file.errorString();
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON format:" << parseError.errorString();
        return;
    }

    QJsonObject obj = doc.object();

    QString militaryUnitNumber = obj.value("number").toString();
    m_dbManager.createMilitaryUnit(militaryUnitNumber);
    m_dbManager.exportDataToJson();
}

void CommandsServer::createDivision() {
    QFile file("C:/Ak-74Utilities/Data/CreateDivisionData.json");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open JSON file:" << file.errorString();
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON format:" << parseError.errorString();
        return;
    }

    QJsonObject obj = doc.object();

    QString militaryUnitId = obj.value("military_unit_id").toString();
    QString divisionName = obj.value("name").toString();

    m_dbManager.createDivision(divisionName, militaryUnitId);
    m_dbManager.exportDataToJson();
}

void CommandsServer::createSubdivision() {
    QFile file("C:/Ak-74Utilities/Data/CreateSubdivisionData.json");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open JSON file:" << file.errorString();
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON format:" << parseError.errorString();
        return;
    }

    QJsonObject obj = doc.object();

    QString subdivisionName = obj.value("name").toString();
    QString divisionId = obj.value("division_id").toString();

    m_dbManager.createSubdivision(subdivisionName, divisionId);
    m_dbManager.exportDataToJson();
}

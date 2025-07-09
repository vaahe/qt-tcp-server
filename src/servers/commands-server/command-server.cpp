#include "commands-server.h"
#include <QFile>
#include <QDebug>

namespace CommandsToSend {
    constexpr const char* FINISHED            = "finished";
    constexpr const char* PROCESSING_STARTED  = "processing_started";
    constexpr const char* CALIBRATION_FAILED  = "calibration_failed";
    constexpr const char* CALIBRATION_STARTED = "calibration_started";
    constexpr const char* UNKNOWN_COMMAND     = "unknown_command";
    constexpr const char* PLAY_SOUND          = "play_sound";
}

namespace CommandsToReceive {
    constexpr const char* SIGN_UP            = "sign_up";
    constexpr const char* SAVE_RESULT        = "save_result";
    constexpr const char* CLOSE_PROCESS      = "close_process";
    constexpr const char* CREATE_DIVISION    = "create_division";
    constexpr const char* CREATE_SUBDIVISION = "create_subdivision";
    constexpr const char* CREATE_MIL_UNIT    = "create_mil_unit";
    constexpr const char* START_PROCESSING   = "start_processing";
    constexpr const char* STOP_PROCESSING    = "stop_processing";
    constexpr const char* START_CALIBRATION  = "start_calibration";
    constexpr const char* PRINT_DATA         = "print_data";
}

CommandsServer::CommandsServer(quint16 port, QObject* parent) :
    BaseTcpServer(port, parent), m_dbManager(DatabaseManager::getInstance())
{
    m_commands = {
        {CommandsToReceive::SIGN_UP,            [this](QTcpSocket*)   { signUp(); }},
        {CommandsToReceive::PRINT_DATA,         [this](QTcpSocket*)   { printData(); }},
        {CommandsToReceive::SAVE_RESULT,        [this](QTcpSocket*)   { saveResult(); }},
        {CommandsToReceive::CLOSE_PROCESS,      [this](QTcpSocket*)   { closeProcess(); }},
        {CommandsToReceive::CREATE_DIVISION,    [this](QTcpSocket*)   { createDivision(); }},
        {CommandsToReceive::CREATE_SUBDIVISION, [this](QTcpSocket*)   { createSubdivision(); }},
        {CommandsToReceive::CREATE_MIL_UNIT,    [this](QTcpSocket*)   { createMilitaryUnit(); }},
        {CommandsToReceive::STOP_PROCESSING,    [this](QTcpSocket*)   { emit stopProcessing(); }},
        {CommandsToReceive::START_PROCESSING,   [this](QTcpSocket* c) { startProcessing(c); }},
        {CommandsToReceive::START_CALIBRATION,  [this](QTcpSocket* c) { startCalibration(c); }},
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
        send(client, CommandsToSend::UNKNOWN_COMMAND);
    }
}

void CommandsServer::startCalibration(QTcpSocket* client) {
    CameraCalibration* calibrator = new CameraCalibration();
    QThread* calibrationThread = new QThread(this);

    calibrator->moveToThread(calibrationThread);

    if (!calibrationThread->isRunning()) {
        calibrationThread->start();
    }

    QObject::connect(calibrator, &CameraCalibration::calibrationSucceeded, this, [this, client]() { send(client, CommandsToSend::FINISHED); });
    QObject::connect(calibrator, &CameraCalibration::calibrationFailed, this, [this, client]() { send(client, CommandsToSend::CALIBRATION_FAILED); });

    QObject::connect(calibrator, &CameraCalibration::calibrationFinished, calibrator, &QObject::deleteLater);
    QObject::connect(calibrator, &CameraCalibration::calibrationFinished, calibrationThread, &QThread::quit);
    QObject::connect(calibrationThread, &QThread::finished, calibrationThread, &QObject::deleteLater);
    QObject::connect(calibrationThread, &QThread::started, calibrator, &CameraCalibration::startCalibration);

    calibrationThread->start();
}

void CommandsServer::startProcessing(QTcpSocket* client) {
    FrameProcessor* frameProcessor = new FrameProcessor();
    QThread* processingThread = new QThread(this);
    HttpClient* httpClient = new HttpClient();

    frameProcessor->moveToThread(processingThread);

    if (!processingThread->isRunning()) {
        httpClient->applyProcessingSettings();
        processingThread->start();
    }

    QObject::connect(frameProcessor, &FrameProcessor::processingStarted, this, [this, client]() { send(client, CommandsToSend::PROCESSING_STARTED); });
    QObject::connect(frameProcessor, &FrameProcessor::processingFinished, processingThread, &QThread::quit);
    QObject::connect(processingThread, &QThread::started, frameProcessor, &FrameProcessor::startProcessing);
    QObject::connect(processingThread, &QThread::finished, frameProcessor, &QObject::deleteLater);
    QObject::connect(processingThread, &QThread::finished, httpClient, &QObject::deleteLater);

    QObject::connect(frameProcessor, &FrameProcessor::pointsSent, this, [this](const QByteArray& data) {
        m_coordinatesServer.sendToAllClients(data);
        m_soundServer.sendToAllClients(CommandsToSend::PLAY_SOUND);
    });

    QObject::connect(this, &CommandsServer::stopProcessing, frameProcessor, &FrameProcessor::stopProcessing);

    processingThread->start();
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

void CommandsServer::saveResult() {
    QFile file("C:/Ak-74Utilities/Data/ResultData.json");

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

    QString userId = obj.value("user_id").toString();
    QString date = obj.value("date").toString();
    QString grade = obj.value("grade").toString();
    QString screenshotFolder = obj.value("screenshot_folder").toString();

    m_dbManager.saveResult(userId, date, grade, screenshotFolder);
    m_dbManager.exportDataToJson();
}

void CommandsServer::printData() {
    // Step 1: Load and parse the JSON
    QFile file("C:/Ak-74Utilities/Data/PrintData.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open PrintData.json";
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonObject filters = doc.object();


    QString queryStr = R"(
        SELECT results.*
        FROM results
        JOIN users ON users.id = results.user_id
        JOIN subdivisions ON subdivisions.id = users.subdivision_id
        JOIN divisions ON divisions.id = subdivisions.division_id
        JOIN military_units ON military_units.id = divisions.military_unit_id
        WHERE 1 = 1
    )";

    QVariantMap params;

    if (filters.contains("unit_id"))
        queryStr += " AND military_units.id = :unit_id", params["unit_id"] = filters["unit_id"].toInt();

    if (filters.contains("division_id"))
        queryStr += " AND divisions.id = :division_id", params["division_id"] = filters["division_id"].toInt();

    if (filters.contains("subdivision_id"))
        queryStr += " AND subdivisions.id = :subdivision_id", params["subdivision_id"] = filters["subdivision_id"].toInt();

    if (filters.contains("user_id"))
        queryStr += " AND users.id = :user_id", params["user_id"] = filters["user_id"].toInt();

    if (filters.contains("dateFrom"))
        queryStr += " AND date(results.date) >= :date_from", params["date_from"] = filters["dateFrom"].toString();

    if (filters.contains("dateTo") && filters["dateTo"].toString() != "//")
        queryStr += " AND date(results.date) <= :date_to", params["date_to"] = filters["dateTo"].toString();

    QSqlQuery query;
    query.prepare(queryStr);
    for (auto it = params.begin(); it != params.end(); ++it)
        query.bindValue(":" + it.key(), it.value());

    if (!query.exec()) {
        qDebug() << "Query failed:" << query.lastError();
        return;
    }

    QFile csvFile("C:/Ak-74Utilities/Data/ExportedResults.csv");
    if (!csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to write ExportedResults.csv";
        return;
    }

    QTextStream out(&csvFile);
    // Header row
    out << "ID,User ID,Date,Grade,Screenshots\n";

    // Data rows
    while (query.next()) {
        out << query.value("id").toInt() << ","
            << query.value("user_id").toInt() << ","
            << "\"" << query.value("date").toString() << "\","  // quote if date has slashes
            << "\"" << query.value("grade").toString() << "\","
            << "\"" << query.value("screenshots").toString().replace("\"", "\"\"") << "\""
            << "\n";
    }

    csvFile.close();
    qDebug() << "Exported to: C:/Ak-74Utilities/Data/ExportedResults.csv";

    PrinterManager::printTextFile("C:/Ak-74Utilities/Data/ExportedResults.csv");
}

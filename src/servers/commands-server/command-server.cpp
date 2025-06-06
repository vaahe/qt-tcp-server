#include "commands-server.h"
#include <QDebug>

CommandsServer::CommandsServer(quint16 port, QObject* parent) :
    BaseTcpServer(port, parent)
{
    QObject::connect(this, &CommandsServer::dataReceived, this, &CommandsServer::onDataReceived);

    if (!connect()) {
        qFatal("CommandsServer failed to listen on port %u", port);
    }
}

CommandsServer::~CommandsServer() {}

void CommandsServer::onDataReceived(QTcpSocket* client, const QByteArray& data) {
    const QString cmd = QString::fromUtf8(data).trimmed();
    qDebug() << "[CommandsServer] Received from" << client->peerAddress().toString() << ":" << cmd;

    if (cmd.compare("start calibration", Qt::CaseInsensitive) == 0) {
        startCalibration(client);
    } else if (cmd.compare("start processing", Qt::CaseInsensitive) == 0) {
        startProcessing(client);
    } else if (cmd.compare("closeProcess", Qt::CaseInsensitive) == 0) {
        closeProcess();
    } else if (cmd.startsWith("print")) {
        const QString filters = cmd.split('-')[1];
        qDebug() << filters;
    } else {
        return;
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

// "closeProcess"

void CommandsServer::printResults(const QString filters) {
    m_dbManager
}

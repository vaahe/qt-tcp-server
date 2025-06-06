#ifndef FRAME_PROCESSOR_H
#define FRAME_PROCESSOR_H

#include "clients/http-client/http-client.h"
#include "utils/globals-manager/globals-manager.h"

#include <QFile>
#include <QMutex>
#include <QThread>
#include <QScreen>
#include <QJsonArray>
#include <QJsonObject>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QGuiApplication>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

class FrameProcessor : public QObject
{
    Q_OBJECT
public:
    explicit FrameProcessor();
    ~FrameProcessor() override;

    void startProcessing();
    void stopProcessing();
    void cropRegionBetweenCapturedPoints(cv::Mat&);
    void loadCalibrationData(const QString& resourcePath);
    void serializePointToByteArray(const cv::Point& point);

signals:
    void processingStarted();
    void processingFinished();
    void pointsSent(const QByteArray& data);
    void frameProcessed(const cv::Mat& processedFrame);
    void trajectoryPointSent(const cv::Point &trajectoryPoint);

private:
    bool m_stopProcessing;
    cv::VideoCapture m_cap;
    cv::Mat m_cameraMatrix;
    cv::Mat m_distortionCoefficients;
    GlobalsManager& globalsManager = GlobalsManager::getInstance();

    HttpClient *m_httpClient;
};

#endif // FRAME_PROCESSOR_H

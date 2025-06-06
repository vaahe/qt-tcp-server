#ifndef CAMERA_CALIBRATION_H
#define CAMERA_CALIBRATION_H

#include "clients/http-client/http-client.h"
#include "utils/globals-manager/globals-manager.h"
#include "utils/display-manager/display-manager.h"

#include <QFile>
#include <QLabel>
#include <QTimer>
#include <QObject>
#include <QThread>
#include <QPixmap>
#include <QMessageBox>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

namespace CalibrationMessages {
    constexpr char FAIL[] = "Calibration failed";
    constexpr char SUCCESS[] = "Calibration successful";
}

class CameraCalibration : public QObject
{
    Q_OBJECT

public:
    CameraCalibration();
    ~CameraCalibration() override;

public:
    void openCamera();

public slots:
    void startCalibration();
    void stopCalibration();
    void onCalibrationTimeout();

signals:
    void calibrationStarted();
    void calibrationFinished();
    void calibrationSucceeded(const QString message);
    void calibrationFailed(const QString message);

    // methods
private:
    void processFrame(const cv::Mat &frame);
    void detectEdges(const cv::Mat& frame);
    void cropRegionBetweenCapturedPoints(const cv::Mat &frame);
    cv::Mat warpFrame(const cv::Mat& frame);
    void showCalibrationImage();
    void hideCalibrationImage();
    void loadCalibrationData(const std::filesystem::path resourcePath);

    // members
private:
    QWidget *m_calibrationImageWidget;
    cv::VideoCapture m_cap;
    bool m_pointsCaptured;
    cv::Point topLeft, topRight, bottomLeft, bottomRight;
    GlobalsManager& globalsManager = GlobalsManager::getInstance();
    DisplayManager displayManager;
    cv::Mat m_cameraMatrix;
    cv::Mat m_distortionCoefficients;
    HttpClient *m_httpClient;
    QTimer *m_timeoutTimer;
};

#endif // CAMERA_CALIBRATION_H

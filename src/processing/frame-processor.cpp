#include "frame-processor.h"

FrameProcessor::FrameProcessor() : m_stopProcessing(false), m_httpClient(new HttpClient()) {}

FrameProcessor::~FrameProcessor() {
    stopProcessing();
    std::cout << "Training worker instance is destoryed" << std::endl;
}


void FrameProcessor::startProcessing() {
    m_cap.open("rtsp://admin:123456789m@192.168.1.64:554/Streaming/Channels/102");

    if (!m_cap.isOpened()) {
        std::cerr << "Error: Could not open video stream." << std::endl;        
        return;
    }

    loadCalibrationData(":data/data/calibration_results.xml");
    m_stopProcessing = false;
    emit processingStarted();

    cv::Mat frame, undistortedFrame, gray;
    while (!m_stopProcessing) {
        m_cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Empty frame captured." << std::endl;
            break;
        }

        cv::undistort(frame, undistortedFrame, m_cameraMatrix, m_distortionCoefficients);

        cropRegionBetweenCapturedPoints(undistortedFrame);

        cv::cvtColor(undistortedFrame, gray, cv::COLOR_BGR2GRAY);
        if (gray.empty()) {
            std::cerr << "Error: Gray frame is empty." << std::endl;
            break;
        }

        emit frameProcessed(gray);

        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(gray, &minVal, &maxVal, &minLoc, &maxLoc);

        if (maxVal > 100) {
            m_cap.read(undistortedFrame);

            serializePointToByteArray(maxLoc);
            emit trajectoryPointSent(maxLoc);
        }
    }

    m_cap.release();
    emit processingFinished();
}


void FrameProcessor::stopProcessing() {
    m_stopProcessing = true;
}

void FrameProcessor::cropRegionBetweenCapturedPoints(cv::Mat &frame) {
    QScreen *screen = QGuiApplication::primaryScreen();
    QSize resolution = screen->geometry().size();
    const int width = resolution.width();
    const int height = resolution.height();

    cv::Point topLeft = globalsManager.get00();
    cv::Point topRight = globalsManager.get01();
    cv::Point bottomLeft = globalsManager.get10();
    cv::Point bottomRight = globalsManager.get11();

    std::vector<cv::Point2f> srcPoints = {topLeft, topRight, bottomRight, bottomLeft};
    std::vector<cv::Point2f> dstPoints = {cv::Point2f(0, 0),cv::Point2f(width - 1, 0),cv::Point2f(width - 1, height - 1),cv::Point2f(0, height - 1)};

    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
    cv::warpPerspective(frame, frame, perspectiveMatrix, cv::Size(width, height));
}

void FrameProcessor::serializePointToByteArray(const cv::Point& point) {
    QString pointString = QString("%1,%2").arg(point.x).arg(point.y);
    QByteArray byteArray = pointString.toUtf8();

    emit pointsSent(byteArray);
}

void FrameProcessor::loadCalibrationData(const QString& resourcePath) {
    cv::FileStorage fs(resourcePath.toStdString(), cv::FileStorage::READ);

    if (!fs.isOpened()) {
        qCritical() << "Error: Could not open the resource file for reading calibration data.";
        exit(-1);
    }

    fs["CameraMatrix"] >> m_cameraMatrix;
    fs["DistortionCoefficients"] >> m_distortionCoefficients;

    if (m_cameraMatrix.empty() || m_distortionCoefficients.empty()) {
        qCritical() << "Error: Calibration data is missing or invalid.";
        exit(-1);
    }

    qDebug() << "Calibration data loaded successfully.";
    fs.release();
}

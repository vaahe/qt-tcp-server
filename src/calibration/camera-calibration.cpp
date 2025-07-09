#include "camera-calibration.h"

CameraCalibration::CameraCalibration() :
    m_calibrationImageWidget(new QWidget()),
    m_cap(cv::VideoCapture()),
    m_pointsCaptured(false),
    m_httpClient(new HttpClient()),
    m_timeoutTimer(new QTimer(this))
{
    qDebug() << "Calib ctor";
    m_timeoutTimer->setSingleShot(true);
    showCalibrationImage();

    connect(m_timeoutTimer, &QTimer::timeout, this, &CameraCalibration::onCalibrationTimeout);
    connect(this, &CameraCalibration::calibrationFinished, this, &CameraCalibration::hideCalibrationImage);
}

CameraCalibration::~CameraCalibration() {
    qDebug() << "Calibration instance is destroyed";
    stopCalibration();
    cv::destroyAllWindows();
}

bool compareY(const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
    return a[0].y < b[0].y;
}

bool compareYY(const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
    return a[0].y > b[0].y;
}

bool compareXX(const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
    return a[0].x < b[0].x;
}

void CameraCalibration::startCalibration() {
    openCamera();

    if (!m_cap.isOpened()) {
        qDebug() << "Error: Camera is not opened";
        emit calibrationFailed("Camera not opened");
        emit calibrationFinished();
        return;
    }

    const std::filesystem::path calibrationFilePath = ":data/data/calibration_results.xml";
    loadCalibrationData(calibrationFilePath);

    emit calibrationStarted();

    m_httpClient->applyCalibrationSettings();
    while (!m_pointsCaptured) {
        cv::Mat frame, undistortedFrame;
        m_cap >> frame;

        if (frame.empty()) {
            qDebug() << "Error: Empty frame captured";
            emit calibrationFailed("Empty frame");
            break;
        }

        cv::undistort(frame, undistortedFrame, m_cameraMatrix, m_distortionCoefficients);
        processFrame(undistortedFrame);

        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    stopCalibration();
    cv::destroyAllWindows();
    emit calibrationFinished();
}

void CameraCalibration::stopCalibration() {
    if (m_cap.isOpened()) {
        m_cap.release();
        qDebug() << "Camera released";
    }
}

void CameraCalibration::openCamera() {
    if (m_cap.isOpened()) {
        qDebug() << "Camera is already opened";
        return;
    }

    bool isConnected = m_cap.open("rtsp://admin:123456789m@192.168.1.64:554/Streaming/Channels/102");

    if (!isConnected || !m_cap.isOpened()) {
        qDebug() << "Error: Couldn't open camera. Please check the connection or the RTSP URL.";
        return;
    }

    qDebug() << "Camera opened successfully";
}


void CameraCalibration::processFrame(const cv::Mat &frame) {
    if (!m_pointsCaptured) {
        detectEdges(frame);
    } else {
        cropRegionBetweenCapturedPoints(frame);
    }
}


void CameraCalibration::detectEdges(const cv::Mat& frame) {
    cv::Mat hsv, blurred;
    cv::GaussianBlur(frame, blurred, cv::Size(5, 5), 0);
    cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);

    cv::Scalar lowerRed1(0, 70, 50);
    cv::Scalar upperRed1(10, 255, 255);
    cv::Scalar lowerRed2(170, 70, 50);
    cv::Scalar upperRed2(180, 255, 255);

    cv::Mat mask1, mask2, redMask;
    cv::inRange(hsv, lowerRed1, upperRed1, mask1);
    cv::inRange(hsv, lowerRed2, upperRed2, mask2);

    redMask = mask1 | mask2;

    cv::imshow("Red mask", redMask);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<std::vector<cv::Point>> sortedContours;
    cv::findContours(redMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.size() == 4) {
        std::vector<std::vector<cv::Point>> minYContours(2);
        std::partial_sort_copy(contours.begin(), contours.end(), minYContours.begin(), minYContours.end(), compareY);
        std::sort(minYContours.begin(), minYContours.end(), compareXX);

        std::vector<std::vector<cv::Point>> maxYContours(2);
        std::partial_sort_copy(contours.begin(), contours.end(), maxYContours.begin(), maxYContours.end(), compareYY);
        std::sort(maxYContours.begin(), maxYContours.end(), compareXX);

        sortedContours.insert(sortedContours.end(), minYContours.begin(), minYContours.end());
        sortedContours.insert(sortedContours.end(), maxYContours.begin(), maxYContours.end());

        int contourCount = 0;
        for (size_t i = 0; i < sortedContours.size(); ++i) {
            cv::Rect boundingBox = cv::boundingRect(sortedContours[i]);
            cv::Point _topLeft = boundingBox.tl();

            if (boundingBox.area() > 100) {
                contourCount++;
                cv::putText(frame, std::to_string(contourCount), _topLeft, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);

                if (i == 0) {
                    topLeft = boundingBox.tl();
                } else if (i == 1) {
                    topRight = cv::Point(boundingBox.br().x, boundingBox.tl().y);
                } else if (i == 2) {
                    bottomLeft = cv::Point(boundingBox.tl().x, boundingBox.br().y);
                } else if (i == 3) {
                    bottomRight = boundingBox.br();
                }
            }
        }

        topLeft.y += 2;
        bottomLeft.x += 3;
        topRight.x -= 2;
        bottomRight.y -= 3;

        globalsManager.set00(topLeft);
        globalsManager.set01(topRight);
        globalsManager.set10(bottomLeft);
        globalsManager.set11(bottomRight);

        qDebug() << topLeft.x << topLeft.y;
        qDebug() << topRight.x << topRight.y;
        qDebug() << bottomLeft.x << bottomLeft.y;
        qDebug() << bottomRight.x << bottomRight.y;

        cv::circle(frame, topLeft, 5, cv::Scalar(0, 0, 255), 3);
        cv::circle(frame, topRight, 5, cv::Scalar(0, 0, 255), 3);
        cv::circle(frame, bottomLeft, 5, cv::Scalar(0, 0, 255), 3);
        cv::circle(frame, bottomRight, 5, cv::Scalar(0, 0, 255), 3);

        cv::imshow("Line 194", frame);
        m_pointsCaptured = true;
        emit calibrationSucceeded(CalibrationMessages::SUCCESS);
        qDebug() << "After calibration successed";
    }
}

void CameraCalibration::cropRegionBetweenCapturedPoints(const cv::Mat &frame) {
    if (!m_pointsCaptured) {
        return;
    }

    std::vector<cv::Point2f> srcPoints = {topLeft, topRight, bottomRight, bottomLeft};

    cv::line(frame, topLeft, topRight, cv::Scalar(0, 0, 255), 2);
    cv::line(frame, topLeft, bottomLeft, cv::Scalar(0, 0, 255), 2);
    cv::line(frame, topRight, bottomRight, cv::Scalar(0, 0, 255), 2);
    cv::line(frame, bottomLeft, bottomRight, cv::Scalar(0, 0, 255), 2);

    cv::Size size(1366 / 2, 768 / 2);
    std::vector<cv::Point2f> dstPoints = {
        cv::Point2f(0, 0),
        cv::Point2f(size.width - 1, 0),
        cv::Point2f(size.width - 1, size.height - 1),
        cv::Point2f(0, size.height - 1)
    };

    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
    cv::Mat warpedFrame;
    cv::warpPerspective(frame, warpedFrame, perspectiveMatrix, size);

    cv::imshow("Frame with lines", frame);
    cv::imshow("Cropped and Warped Frame", warpedFrame);
}

cv::Mat CameraCalibration::warpFrame(const cv::Mat& frame)
{
    if (frame.empty()) {
        std::cerr << "Error: Unable to read input image." << std::endl;
    }

    cv::Mat warpedFrame;

    std::vector<cv::Point2f> srcPoints{topLeft, topRight, bottomRight, bottomLeft};

    float maxX = 0, maxY = 0;
    for (const auto& point : srcPoints)
    {
        if (point.x > maxX)
        {
            maxX = point.x;
        }
        if (point.y > maxY)
        {
            maxY = point.y;
        }
    }

    std::vector<cv::Point2f> dstPoints{
        cv::Point2f(0, 0),
        cv::Point2f(maxX, 0),
        cv::Point2f(maxX, maxY),
        cv::Point2f(0, maxY)
    };

    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
    cv::Size warpedSize(static_cast<int>(maxX), static_cast<int>(maxY));
    cv::warpPerspective(frame, warpedFrame, perspectiveMatrix, warpedSize);

    return warpedFrame;
}

void CameraCalibration::showCalibrationImage() {
    if (displayManager.hasMultipleDisplays()) {
        m_calibrationImageWidget->setWindowTitle("Calibration Image");
        m_calibrationImageWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        m_calibrationImageWidget->setAttribute(Qt::WA_TranslucentBackground);
        m_calibrationImageWidget->setAttribute(Qt::WA_NoSystemBackground, false);

        const QString backgroundImagePath = ":/images/images/background1.png";
        QPixmap pixmap(backgroundImagePath);

        if (pixmap.isNull()) {
            qDebug() << "Error: Couldn't open calibration image";
            delete m_calibrationImageWidget;
            return;
        }

        QSize displaySize = displayManager.secondaryDisplaySize();

        QLabel* backgroundLabel = new QLabel(m_calibrationImageWidget);
        backgroundLabel->setPixmap(pixmap);
        backgroundLabel->setScaledContents(true);
        backgroundLabel->setGeometry(0, 0, displaySize.width(), displaySize.height());

        displayManager.showOnSecondaryDisplay(m_calibrationImageWidget);
    } else {
        qDebug() << "Only one display detected. Unable to show on secondary display.";
    }
}

void CameraCalibration::hideCalibrationImage() {
    if (m_pointsCaptured) {
        m_calibrationImageWidget->hide();
    }
}

void CameraCalibration::loadCalibrationData(const std::filesystem::path resourcePath) {
    QFile file(resourcePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Error: Could not open the resource file for reading calibration data.";
        exit(-1);
    }

    QByteArray fileData = file.readAll();
    cv::FileStorage fs(fileData.toStdString(), cv::FileStorage::READ | cv::FileStorage::MEMORY);

    if (!fs.isOpened()) {
        qCritical() << "Error: Could not open the file storage for reading calibration data.";
        exit(-1);
    }

    fs["CameraMatrix"] >> m_cameraMatrix;
    fs["DistortionCoefficients"] >> m_distortionCoefficients;

    if (m_cameraMatrix.empty() || m_distortionCoefficients.empty()) {
        qCritical() << "Error: Calibration data is missing or invalid.";
        exit(-1);
    }

    qDebug() << "Calibration data loaded successfully.";

    file.close();
}


void CameraCalibration::onCalibrationTimeout() {
    if (!m_pointsCaptured) {
        qWarning() << "Calibration timed out after 30 seconds.";
        stopCalibration();
        emit calibrationFailed("Timeout");
        // restart
        startCalibration();
    }
}

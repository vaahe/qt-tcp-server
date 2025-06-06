#include "http-client.h"
#include <QTimer>

HttpClient::HttpClient(QObject *parent) :
    QObject(parent),
    m_manager(new QNetworkAccessManager(this)),
    m_username("admin"),
    m_password("123456789m") {}

void HttpClient::response() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (reply) {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Request failed:" << reply->errorString();
        } else {
            QByteArray response = reply->readAll();
            qDebug() << "Response:" << response;
        }

        reply->deleteLater();
    }
}

void HttpClient::put(const QString& url, const QByteArray& xmlData) {
    QUrl requestUrl(m_cameraIp + url);
    QNetworkRequest request(requestUrl);

    QString authHeader = createBasicAuthHeader();
    request.setRawHeader("Authorization", authHeader.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");

    QNetworkReply* authenticatedReply = m_manager->put(request, xmlData);
    connect(authenticatedReply, &QNetworkReply::finished, this, &HttpClient::response);
}

QString HttpClient::createBasicAuthHeader() {
    QByteArray auth = m_username.toUtf8() + ":" + m_password.toUtf8();
    QByteArray authBase64 = auth.toBase64();

    return "Basic " + authBase64;
}

void HttpClient::setCameraDaytime(const QString &daytime) {
    QByteArray xmlData = QString(R"(
            <IrcutFilter>
                <IrcutFilterType>%1</IrcutFilterType>
                <nightToDayFilterLevel>4</nightToDayFilterLevel>
                <nightToDayFilterTime>5</nightToDayFilterTime>
            </IrcutFilter>
        )").arg(daytime).toUtf8();

    put("/ISAPI/Image/channels/1/ircutFilter/", xmlData);

    qDebug() << "line 54";
}

void HttpClient::setCameraExposure(const QString &exposition) {
    QByteArray expositionXmlData = QString(R"(
        <Shutter><ShutterLevel>%1</ShutterLevel></Shutter>
        )").arg(exposition).toUtf8();

    put("/ISAPI/Image/channels/1/shutter/", expositionXmlData);


    qDebug() << "line 65";
}

void HttpClient::setCameraColorSettings(const QString &brightness, const QString &contrast, const QString &saturation) {
    QByteArray colorXmlData = QString(R"(
        <Color>
            <brightnessLevel>%1</brightnessLevel>
            <contrastLevel>%2</contrastLevel>
            <saturationLevel>%3</saturationLevel>
        </Color>
    )").arg(brightness, contrast, saturation).toUtf8();

    put("/ISAPI/Image/channels/1/color/", colorXmlData);


    qDebug() << "line 80";
}


void HttpClient::setCameraSharpness(const QString &sharpness) {
    QByteArray sharpnessXmlData = QString(R"(
        <Sharpness>
            <SharpnessLevel>%1</SharpnessLevel>
        </Sharpness>
    )").arg(sharpness).toUtf8();

    put("/ISAPI/Image/channels/1/sharpness/", sharpnessXmlData);

    qDebug() << "line 93";
}

void HttpClient::applyCalibrationSettings() {
    setCameraExposure("1/6");

    QTimer::singleShot(500, [this]() {
        setCameraColorSettings("100", "75", "100");
    });

    QTimer::singleShot(1000, [this]() {
        setCameraSharpness("0");
    });

    QTimer::singleShot(1500, [this]() {
        setCameraDaytime("day");
    });
}

void HttpClient::applyProcessingSettings() {
    setCameraDaytime("night");

    QTimer::singleShot(500, [this]() {
        setCameraColorSettings("0", "70", "0");
    });

    QTimer::singleShot(1000, [this]() {
        setCameraExposure("1/120");
    });
}

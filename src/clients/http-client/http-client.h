#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <QUrl>
#include <QObject>
#include <QByteArray>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class HttpClient : public QObject {
    Q_OBJECT

public:
    explicit HttpClient(QObject *parent = nullptr);

    void setCameraDaytime(const QString &daytime);
    void setCameraExposure(const QString &exposition);
    void setCameraColorSettings(const QString &brightness, const QString &contrast, const QString &saturation);
    void setCameraSharpness(const QString &sharpness);

    void applyCalibrationSettings();
    void applyProcessingSettings();

private:
    QNetworkAccessManager *m_manager;
    QString m_username;
    QString m_password;
    QString m_cameraIp = "http://192.168.1.64";

    void put(const QString& url, const QByteArray& xmlData);
    void response();

    QString createBasicAuthHeader();
};

#endif // HTTP_CLIENT_H

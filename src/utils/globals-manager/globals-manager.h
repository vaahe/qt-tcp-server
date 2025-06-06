#ifndef GLOBALS_MANAGER_H
#define GLOBALS_MANAGER_H

#include <QDebug>
#include <QString>
#include <QObject>
#include <QStringList>
#include <opencv2/core.hpp>

class GlobalsManager : public QObject {
    Q_OBJECT

signals:

public:
    static GlobalsManager& getInstance() {
        static GlobalsManager instance;
        return instance;
    }

    void setLanguage(const QString& language) {
        m_language = language;
        qDebug() << language;
    }

    QString getLanguage() {
        return m_language;
    }

    void setComPort(const QString& comPort) {
        m_comPort = comPort;
    }

    QString getComPort() {
        return m_comPort;
    }

    void set00(cv::Point new00)
    {
        m00 = new00;
    }

    cv::Point get00()
    {
        return m00;
    }

    void set01(cv::Point new01)
    {
        m01 = new01;
    }

    cv::Point get01()
    {
        return m01;
    }

    void set10(cv::Point new10)
    {
        m10 = new10;
    }

    cv::Point get10()
    {
        return m10;
    }

    void set11(cv::Point new11)
    {
        m11 = new11;
    }

    cv::Point get11()
    {
        return m11;
    }

private:
    GlobalsManager() {}

private:
    QString m_comPort;
    QString m_language;

    cv::Point m00;
    cv::Point m01;
    cv::Point m10;
    cv::Point m11;
};

#endif // GLOBALS_MANAGER_H

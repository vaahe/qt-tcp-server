#ifndef RESULT_H
#define RESULT_H

#include <QList>
#include <QString>

class Result {
public:
    Result() = default;
    Result(QString id, QString user_id, QString grade, QString date, QList<QString> screenshots)
        : m_id(id), m_user_id(user_id), m_grade(grade), m_date(date), m_screenshots(screenshots) {}
    ~Result();

private:
    QString m_id;
    QString m_user_id;
    QString m_grade;
    QString m_date;
    QList<QString> m_screenshots;
};

#endif // RESULT_H

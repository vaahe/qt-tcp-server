#ifndef DIVISION_H
#define DIVISION_H

#include <QString>

class Division {
public:
    Division() = default;
    Division(QString id, QList<QString> subdivision_ids, QString name) : m_id(id), m_subdivision_ids(subdivision_ids), m_name(name) {}
    ~Division();

private:
    QString m_id;
    QList<QString> m_subdivision_ids;
    QString m_name;
};

#endif // DIVISION_H

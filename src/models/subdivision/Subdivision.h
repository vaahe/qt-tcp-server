#ifndef SUBDIVISION_H
#define SUBDIVISION_H

#include <QList>
#include <QString>

#include "models/user/User.h"

class Subdivision {
public:
    Subdivision() = default;
    Subdivision(QString id, QString name, QString division_id, QList<User> users)
        : m_id(id), m_name(name), m_division_id(division_id), m_users(users) {}
    ~Subdivision();

private:
    QString m_id;
    QString m_name;
    QString m_division_id;
    QList<User> m_users;
};

#endif // SUBDIVISION_H

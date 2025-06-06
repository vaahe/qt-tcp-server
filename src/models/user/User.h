#ifndef USER_H
#define USER_H

#include "models/result/Result.h"

#include <QString>

class User {
public:
    User() = default;
    User(QString id, QString fullName, QString rank, QString position, QString division_id, QString subdivision_id, QList<Result> results)
        : m_id(id), m_full_name(fullName), m_rank(rank), m_position(position), m_division_id(division_id), m_subdivision_id(subdivision_id), m_results(results) {}
    ~User();

private:
    QString m_id;
    QString m_full_name;
    QString m_rank;
    QString m_position;
    QString m_division_id;
    QString m_subdivision_id;
    QList<Result> m_results;
};

#endif // USER_H

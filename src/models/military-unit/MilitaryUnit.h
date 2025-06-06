#ifndef MILITARYUNIT_H
#define MILITARYUNIT_H

#include <QString>

class MilitaryUnit {
public:
    MilitaryUnit() = default;
    MilitaryUnit(QString id, QString number) : m_id(id), m_number(number) {}
    ~MilitaryUnit();

private:
    QString m_id;
    QString m_number;
};

#endif // MILITARYUNIT_H

#ifndef USER_H
#define USER_H

#include "result.h"

#include <QString>

struct User {
    QString id;
    QString fullName;
    QString militaryUnit;
    QString subdivision;
    Result results;
};

#endif // USER_H

 #ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "models/user/User.h"
#include "models/result/Result.h"
#include "models/division/Division.h"
#include "models/subdivision/Subdivision.h"
#include "models/military-unit/MilitaryUnit.h"

#include <QDir>
#include <QDate>
#include <QUuid>
#include <QObject>
#include <QStringList>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <QSqlRecord>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    struct SignUpData {
        const QString& fullName;
        const QString& rank;
        const QString& position;
        const QString& subDivisionId;
    };

    struct QueryFilter {
        QString militaryUnitId;
        QString divisionId;
        QString subdivisionId;
        QString userId;
        QDate dateFrom;
        QDate dateTo;
    };

public:
    static DatabaseManager& getInstance();

    bool connect();
    void disconnect();

public:
    bool isConnected() const;
    bool insertResult(const Result& result);
    bool signUp(QString fullName, QString rank, QString position, QString subdivision_id);

    bool createDivision(const QString divisionName, const QString militaryUnitId);
    bool createSubdivision(const QString subdivisionName, const QString divisionId);
    bool createMilitaryUnit(const QString militaryUnitNumber);

    void seedMockData();
    // bool exportDataToCSV(const QString& filePath, const QueryFilter& filter = QueryFilter());
    void exportDataToJson(const QString &filePath = "C:/Ak-74Utilities/Data/MainData.json");

    QSqlQuery getResults(const QueryFilter& filter);
    QSqlQuery getDivisions();
    QSqlQuery getSubdivisions();
    QSqlQuery getUsersBySubdivision(const QString& subdivisionId);

    void createUsersTable();
    void createResultsTable();
    void createDivisionsTable();
    void createSubdivisionsTable();
    void createMilitaryUnitsTable();
    bool createTable(const QString& tableName, const QString& tableDefinition);

private:
    DatabaseManager();
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    bool m_isConnected;

public:
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H

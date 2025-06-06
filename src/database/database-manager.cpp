#include "database-manager.h"

DatabaseManager::DatabaseManager() : m_isConnected(false) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("database.db");

    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA foreign_keys = ON;");
}

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::createTable(const QString &tableName, const QString &tableDefinition) {
    QSqlQuery query;
    QString queryStr = QString("CREATE TABLE IF NOT EXISTS %1 (%2);").arg(tableName, tableDefinition);

    if (!query.exec(queryStr)) {
        qDebug() << "Error creating table:" << query.lastError().text();
        return false;
    }

    return true;
}

void DatabaseManager::createUsersTable() {
    const QString tableName = "users";
    const QString tableDefinition =
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "full_name TEXT NOT NULL, "
        "rank TEXT, "
        "position TEXT, "
        "subdivision_id INTEGER, "
        "FOREIGN KEY(subdivision_id) REFERENCES subdivisions(id) ON DELETE CASCADE";

    createTable(tableName, tableDefinition);
}

void DatabaseManager::createDivisionsTable() {
    const QString tableName = "divisions";
    const QString tableDefinition = "id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL UNIQUE, military_unit_id TEXT NOT NULL, FOREIGN KEY(military_unit_id) REFERENCES military_units(id)";

    createTable(tableName, tableDefinition);
}

void DatabaseManager::createSubdivisionsTable() {
    const QString tableName = "subdivisions";
    const QString tableDefinition =
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "division_id INTEGER NOT NULL,"
        "FOREIGN KEY(division_id) REFERENCES divisions(id) "
        "ON DELETE CASCADE ON UPDATE CASCADE,"
        "UNIQUE(name, division_id)";

    createTable(tableName, tableDefinition);
}

void DatabaseManager::createResultsTable() {
    const QString tableName = "results";
    const QString tableDefinition =
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "date TEXT,"
        "grade TEXT,"
        "screenshots TEXT,"
        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE";

    createTable(tableName, tableDefinition);
}

void DatabaseManager::createMilitaryUnitsTable() {
    const QString tableName = "military_units";
    const QString tableDefinition = "id INTEGER PRIMARY KEY AUTOINCREMENT, number INTEGER NOT NULL";

    createTable(tableName, tableDefinition);
}


bool DatabaseManager::connect() {
    if (!m_db.open()) {
        return false;
    }

    qDebug() << "Connected to db";
    m_isConnected = true;

    createUsersTable();
    createResultsTable();
    createDivisionsTable();
    createSubdivisionsTable();
    createMilitaryUnitsTable();

    return true;
}

void DatabaseManager::disconnect() {
    if (m_db.open()) {
        m_db.close();
    }
}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

QSqlQuery DatabaseManager::getDivisions() {
    QSqlQuery query;
    const QString queryStr = "SELECT * FROM divisions";

    if (!query.exec(queryStr)) {
        qDebug() << "Error while getting divisions:" << query.lastError().text();
    }

    return query;
}

QSqlQuery DatabaseManager::getSubdivisions() {
    QSqlQuery query;
    const QString queryStr = "SELECT * FROM subdivisions";

    if (!query.exec(queryStr)) {
        qDebug() << "Error while getting subdivisions" << query.lastError().text();
    }

    return query;
}

bool DatabaseManager::isConnected() const {
    return m_isConnected;
}

void DatabaseManager::seedMockData() {
    QSqlQuery q;

    // 0) Enable foreign keys once per connection
    {
        QSqlQuery pragma;
        pragma.exec("PRAGMA foreign_keys = ON;");
    }

    // 1) Military Units
    {
        bool ok = q.exec(
            "INSERT INTO military_units(id, number) VALUES "
            "(1, 101), "
            "(2, 202);"
            );
        if (!ok) {
            qDebug() << "[ERROR] Seeding military_units failed:" << q.lastError().text();
        } else {
            qDebug() << "[DEBUG] Seeding military_units succeeded.";
        }
    }

    // 2) Divisions
    {
        bool ok = q.exec(
            "INSERT INTO divisions(id, name, military_unit_id) VALUES "
            "(1, '1st Infantry Division', 1), "
            "(2, '2nd Armored Division', 2);"
            );
        if (!ok) {
            qDebug() << "[ERROR] Seeding divisions failed:" << q.lastError().text();
        } else {
            qDebug() << "[DEBUG] Seeding divisions succeeded.";
        }
    }

    // 3) Subdivisions
    {
        bool ok = q.exec(
            "INSERT INTO subdivisions(id, name, division_id) VALUES "
            "(1, '1st Brigade Combat Team', 1), "
            "(2, '2nd Brigade Combat Team', 1), "
            "(3, '1st Armored Brigade', 2), "
            "(4, '2nd Armored Brigade', 2);"
            );
        if (!ok) {
            qDebug() << "[ERROR] Seeding subdivisions failed:" << q.lastError().text();
        } else {
            qDebug() << "[DEBUG] Seeding subdivisions succeeded.";
        }
    }

    // 4) Users
    {
        bool ok = q.exec(
            "INSERT INTO users(id, full_name, rank, position, subdivision_id) VALUES "
            "(1, 'Sargis Sargsyan Sargsi', 'Captain', 'Company Commander', 1), "
            "(2, 'John Doe Smith', 'Major', 'Battalion Commander', 2), "
            "(3, 'Alice Johnson A.', 'Lieutenant','Platoon Leader', 3), "
            "(4, 'Bob Williams B.', 'Sergeant', 'Squad Leader', 4);"
            );
        if (!ok) {
            qDebug() << "[ERROR] Seeding users failed:" << q.lastError().text();
        } else {
            qDebug() << "[DEBUG] Seeding users succeeded.";
        }
    }

    // 5) Results
    {
        // We must INSERT into (date, grade, screenshots). Using a raw‐string literal
        // ensures that the double‐quotes inside JSON are not mangled.
        bool ok = q.exec(R"(
            INSERT INTO results(id, user_id, date, grade, screenshots) VALUES
              (1, 1, '2023-01-15', 95, '["screenshot1.png","screenshot2.png"]'),
              (2, 1, '2023-02-20', 88, '["screenshot3.png"]'),
              (3, 2, '2023-03-10', 92, '["screenshot4.png"]'),
              (4, 3, '2023-04-05', 85, '[]'),
              (5, 4, '2023-05-01', 90, '["screenshot5.png"]');
        )");
        if (!ok) {
            qDebug() << "[ERROR] Seeding results failed:" << q.lastError().text();
        } else {
            qDebug() << "[DEBUG] Seeding results succeeded.";
        }
    }
}

void DatabaseManager::exportDataToJson(const QString &filePath) {
    // Enable foreign keys (just once per connection)
    {
        QSqlQuery pragmaQuery;
        pragmaQuery.exec("PRAGMA foreign_keys = ON;");
    }

    // Check row count first
    {
        QSqlQuery countQuery("SELECT COUNT(*) FROM military_units");
        if (countQuery.next()) {
            int cnt = countQuery.value(0).toInt();
            qDebug() << "[DEBUG] Before export: military_units row count =" << cnt;
        }
    }

    // 1) Query all military units
    QSqlQuery militaryUnitQuery;
    if (!militaryUnitQuery.exec("SELECT id, number FROM military_units")) {
        qDebug() << "[ERROR] Querying military_units failed:" << militaryUnitQuery.lastError().text();
    }

    QJsonArray militaryUnitsArray;
    int         unitCounter = 0;

    while (militaryUnitQuery.next()) {
        unitCounter++;
        const QString unitId     = militaryUnitQuery.value("id").toString();
        const int     unitNumber = militaryUnitQuery.value("number").toInt();

        qDebug() << "[DEBUG] Exporting unit #" << unitCounter << " id=" << unitId << " number=" << unitNumber;

        QJsonObject militaryUnitObj;
        militaryUnitObj["id"]     = unitId;
        militaryUnitObj["number"] = unitNumber;

        // 2) Query divisions for this unit
        QSqlQuery divQuery;
        divQuery.prepare("SELECT id, name FROM divisions WHERE military_unit_id = :unitId");
        divQuery.bindValue(":unitId", unitId);
        if (!divQuery.exec()) {
            qDebug() << "[ERROR] Querying divisions failed:" << divQuery.lastError().text();
        }

        QJsonArray divisionsArray;
        int         divCounter = 0;

        while (divQuery.next()) {
            divCounter++;
            const QString divId   = divQuery.value("id").toString();
            const QString divName = divQuery.value("name").toString();

            qDebug() << "  [DEBUG] Unit" << unitId << " → Division #" << divCounter << " id=" << divId << " name=" << divName;

            QJsonObject divObj;
            divObj["id"]   = divId;
            divObj["name"] = divName;

            // 3) Query subdivisions for this division
            QSqlQuery subQuery;
            subQuery.prepare("SELECT id, name FROM subdivisions WHERE division_id = :divId");
            subQuery.bindValue(":divId", divId);
            if (!subQuery.exec()) {
                qDebug() << "[ERROR] Querying subdivisions failed:" << subQuery.lastError().text();
            }

            QJsonArray subsArray;
            int         subCounter = 0;

            while (subQuery.next()) {
                subCounter++;
                const QString subId   = subQuery.value("id").toString();
                const QString subName = subQuery.value("name").toString();

                qDebug() << "    [DEBUG] Division" << divId << " → Subdivision #" << subCounter << " id=" << subId << " name=" << subName;

                QJsonObject subObj;
                subObj["id"]   = subId;
                subObj["name"] = subName;

                // 4) Query users for this subdivision
                QSqlQuery userQuery;
                userQuery.prepare(
                    "SELECT id, full_name, rank, position "
                    "FROM users "
                    "WHERE subdivision_id = :subId"
                    );
                userQuery.bindValue(":subId", subId);
                if (!userQuery.exec()) {
                    qDebug() << "[ERROR] Querying users failed:" << userQuery.lastError().text();
                }

                QJsonArray usersArray;
                int         userCounter = 0;

                while (userQuery.next()) {
                    userCounter++;
                    const QString userId   = userQuery.value("id").toString();
                    const QString userFullName = userQuery.value("full_name").toString();
                    const QString rank     = userQuery.value("rank").toString();
                    const QString position = userQuery.value("position").toString();

                    qDebug() << "[DEBUG] Subdivision" << subId << " → User #" << userCounter << " id=" << userId << "full name=" << userFullName;

                    QJsonObject userObj;
                    userObj["id"]        = userId;
                    userObj["full_name"] = userFullName;
                    userObj["rank"]      = rank;
                    userObj["position"]  = position;

                    // 5) Query results for this user
                    QSqlQuery resultQuery;
                    resultQuery.prepare(
                        "SELECT date, grade, screenshots "
                        "FROM results "
                        "WHERE user_id = :userId"
                        );
                    resultQuery.bindValue(":userId", userId);
                    if (!resultQuery.exec()) {
                        qDebug() << "[ERROR] Querying results failed:" << resultQuery.lastError().text();
                    }

                    QJsonArray resultsArray;
                    int         resCounter = 0;

                    while (resultQuery.next()) {
                        resCounter++;
                        const QString dateStr  = resultQuery.value("date").toString();
                        const int     grade    = resultQuery.value("grade").toInt();
                        const QString shotsRaw = resultQuery.value("screenshots").toString();

                        qDebug() << "        [DEBUG] User" << userId << " → Result #" << resCounter << " date=" << dateStr << " grade=" << grade;

                        // Parse screenshots
                        QJsonArray screenshotsArray;
                        QJsonDocument shotsDoc = QJsonDocument::fromJson(shotsRaw.toUtf8());
                        if (shotsDoc.isArray()) {
                            screenshotsArray = shotsDoc.array();
                        } else {
                            screenshotsArray.append(shotsRaw);
                        }

                        QJsonObject resultObj;
                        resultObj["date"]        = dateStr;
                        resultObj["grade"]       = grade;
                        resultObj["screenshots"] = screenshotsArray;
                        resultsArray.append(resultObj);
                    }

                    userObj["results"] = resultsArray;
                    usersArray.append(userObj);
                }

                subObj["users"] = usersArray;
                subsArray.append(subObj);
            }

            divObj["subdivisions"] = subsArray;
            divisionsArray.append(divObj);
        }

        militaryUnitObj["divisions"] = divisionsArray;
        militaryUnitsArray.append(militaryUnitObj);
    }

    // 6) Wrap under “military_units”
    QJsonObject rootObj;
    rootObj["military_units"] = militaryUnitsArray;

    QJsonDocument doc(rootObj);
    QByteArray    jsonBytes = doc.toJson(QJsonDocument::Indented);

    // 7) Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "[ERROR] Failed to open file for writing:" << filePath;
    }
    file.write(jsonBytes);
    file.close();

    qDebug() << "[DEBUG] Successfully written JSON to" << filePath << " (military_units count =" << militaryUnitsArray.size() << ")";

}

bool DatabaseManager::signUp(const SignUpData& signUpData) {
    QSqlQuery query;
    const QString queryStr = "INSERT INTO users VALUES (:full_name, :rank, :position, :subdivision_id)";
    query.bindValue(":full_name", signUpData.fullName);
    query.bindValue(":rank", signUpData.rank);
    query.bindValue(":position", signUpData.position);
    query.bindValue("subdivision_id", signUpData.subDivisionId);

    if (!query.exec(queryStr)) {
        qDebug() << "Error while signing up:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::exportDataToCSV(const QString filePath, const QueryFilter& filter) {
    QSqlQuery query = this->getResults(filter);
    if (query.lastError().isValid()) {
        qDebug() << "[ERROR] exportDataToCSV: failed to fetch filtered data:"
                 << query.lastError().text();
        return false;
    }

    // 2) Open (or create/truncate) the output file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qDebug() << "[ERROR] exportDataToCSV: cannot open file for writing:"
                 << filePath;
        return false;
    }
    QTextStream out(&file);

    // 3) Write the CSV header
    //    These column names must match the SELECT in getResults():
    //    (r.id AS result_id, r.user_id AS user_id, r.date, r.grade, r.screenshots)
    out << "result_id,user_id,date,grade,screenshots\n";

    // 4) Loop through each row and write one CSV line
    while (query.next()) {
        int     resultId    = query.value("result_id").toInt();
        int     userId      = query.value("user_id").toInt();
        QString dateStr     = query.value("date").toString();        // "YYYY-MM-DD"
        int     grade       = query.value("grade").toInt();
        QString screenshots = query.value("screenshots").toString(); // e.g. '["s1.png","s2.png"]'

        // 4a) Escape the TEXT fields
        //     - date is safe as “YYYY-MM-DD” but we’ll quote it for consistency
        QString dateField   = escapeForCsv(dateStr);
        QString shotsField  = escapeForCsv(screenshots);

        // 4b) Write comma-separated values
        out
            << resultId   << ","
            << userId     << ","
            << dateField  << ","
            << grade      << ","
            << shotsField << "\n";
    }

    file.close();
    qDebug() << "[DEBUG] exportDataToCSV: Successfully wrote CSV to" << filePath;
    return true;
}

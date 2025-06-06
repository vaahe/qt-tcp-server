#include "json-parser.h"

#include <QFile>
#include <QJsonParseError>

JsonParser::JsonParser(QObject* parent)
    : QObject(parent)
{
}

JsonParser::~JsonParser() = default;

bool JsonParser::parseFile(const QString& filePath,
                           QJsonDocument &jsonDoc,
                           QString &errorString)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorString = QStringLiteral("Failed to open file: %1").arg(filePath);
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    return parseString(QString::fromUtf8(data), jsonDoc, errorString);
}

bool JsonParser::parseString(const QString& jsonString,
                             QJsonDocument &jsonDoc,
                             QString &errorString)
{
    QJsonParseError parseError;
    jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        errorString = parseError.errorString();
        return false;
    }
    return true;
}

QJsonObject JsonParser::toObject(const QJsonDocument& jsonDoc)
{
    if (jsonDoc.isObject()) {
        return jsonDoc.object();
    }
    return QJsonObject();
}

QJsonArray JsonParser::toArray(const QJsonDocument& jsonDoc)
{
    if (jsonDoc.isArray()) {
        return jsonDoc.array();
    }
    return QJsonArray();
}

bool JsonParser::writeJsonToFile(const QString &filePath,
                                 const QJsonObject &root,
                                 QString &errorString)
{
    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorString = QStringLiteral("Cannot open file for writing: %1").arg(filePath);
        return false;
    }
    qint64 written = file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    if (written <= 0) {
        errorString = QStringLiteral("Failed to write JSON to %1").arg(filePath);
        return false;
    }
    return true;
}


DatabaseManager::QueryFilter JsonParser::parseQueryFilter(const QString &filePath) {
    DatabaseManager::QueryFilter filter;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[ERROR] parseQueryFilterFromJson: cannot open file:" << filePath;
        return filter; // all fields remain -1
    }

    QByteArray raw = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "[ERROR] parseQueryFilterFromJson: JSON parse failed:" << parseError.errorString();
        return filter;
    }

    if (!doc.isObject()) {
        qDebug() << "[ERROR] parseQueryFilterFromJson: JSON root is not an object";
        return filter;
    }

    QJsonObject obj = doc.object();

    if (obj.contains("military_unit_id") && obj.value("military_unit_id").isDouble()) {
        int muid = obj.value("military_unit_id").toInt(-1);
        if (muid >= 0) filter.militaryUnitId = muid;
    }

    if (obj.contains("division_id") && obj.value("division_id").isDouble()) {
        int did = obj.value("division_id").toInt(-1);
        if (did >= 0) filter.divisionId = did;
    }

    if (obj.contains("subdivision_id") && obj.value("subdivision_id").isDouble()) {
        int sid = obj.value("subdivision_id").toInt(-1);
        if (sid >= 0) filter.subdivisionId = sid;
    }

    if (obj.contains("user_id") && obj.value("user_id").isDouble()) {
        int uid = obj.value("user_id").toInt(-1);
        if (uid >= 0) filter.userId = uid;
    }

    if (obj.contains("date_from") && obj.contains("date_to")) {
        const QDate dateTo = obj.value("data_to");
        const QDate dateFrom = obj.value("date_from");

        filter.dateTo = dateTo;
        filter.dateFrom = dateFrom;
    }

    return filter;
}

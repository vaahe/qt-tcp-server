#ifndef JSONPARSER_H
#define JSONPARSER_H

#include "database/database-manager.h"

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

/**
 * @brief Utility class for parsing JSON content using Qt's JSON framework.
 */
class JsonParser : public QObject {
    Q_OBJECT

public:
    explicit JsonParser(QObject* parent = nullptr);
    ~JsonParser() override;

    /**
     * @brief Parse JSON from a file.
     * @param filePath Path to the JSON file.
     * @param jsonDoc Output QJsonDocument on success.
     * @param errorString Error message on failure.
     * @return true if parsing succeeded, false otherwise.
     */
    static bool parseFile(const QString& filePath, QJsonDocument &jsonDoc, QString &errorString);

    /**
     * @brief Parse JSON from a raw string.
     * @param jsonString A QString containing JSON text.
     * @param jsonDoc Output QJsonDocument on success.
     * @param errorString Error message on failure.
     * @return true if parsing succeeded, false otherwise.
     */
    static bool parseString(const QString& jsonString, QJsonDocument &jsonDoc, QString &errorString);

    /**
     * @brief Extract a QJsonObject from a QJsonDocument.
     * @param jsonDoc The document to extract from.
     * @return QJsonObject if the document holds an object, empty otherwise.
     */
    static QJsonObject toObject(const QJsonDocument& jsonDoc);

    /**
     * @brief Extract a QJsonArray from a QJsonDocument.
     * @param jsonDoc The document to extract from.
     * @return QJsonArray if the document holds an array, empty otherwise.
     */
    static QJsonArray toArray(const QJsonDocument& jsonDoc);

    /**
     * @brief               Write a QJsonObject out to disk.
     * @param filePath      Where to save the JSON.
     * @param root          The object to serialize.
     * @param errorString   On failure, filled with the error.
     * @return              true on success.
     */
    static bool writeJsonToFile(const QString &filePath, const QJsonObject &root, QString &errorString);

    static DatabaseManager::QueryFilter parseQueryFilter(const QString& filePath, QJsonDocument &jsonDoc, QString &errorString);
};

#endif // JSONPARSER_H

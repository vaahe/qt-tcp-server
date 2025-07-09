#ifndef PRINTERMANAGER_H
#define PRINTERMANAGER_H

#include <QString>

class PrinterManager {
public:
    static void printTextFile(const QString& filePath);
};

#endif // PRINTERMANAGER_H

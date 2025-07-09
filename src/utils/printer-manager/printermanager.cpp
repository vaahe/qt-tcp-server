#include "PrinterManager.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QTextStream>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

void PrinterManager::printTextFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for printing:" << filePath;
        return;
    }

    QTextStream stream(&file);
    QString content = stream.readAll();
    file.close();

    QPrinter printer;
    QPrintDialog printDialog(&printer);
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        if (!painter.begin(&printer)) {
            qWarning() << "Failed to start printer";
            return;
        }

        QRect rect = painter.viewport();
        QFont font("Courier", 10);
        painter.setFont(font);

        int lineSpacing = QFontMetrics(font).lineSpacing();
        int y = 0;
        for (const QString& line : content.split('\n')) {
            painter.drawText(0, y, rect.width(), lineSpacing, Qt::AlignLeft, line);
            y += lineSpacing;

            if (y > rect.height()) {
                printer.newPage();
                y = 0;
            }
        }

        painter.end();
    }
}

#include "debug-widget.h"

#include <QFrame>
#include <QLabel>
#include <QColor>
#include <QDateTime>
#include <QHBoxLayout>


DebugWidget::DebugWidget(QWidget* parent)
    : QWidget(parent),
    m_scrollArea(new QScrollArea(this)),
    m_container(new QWidget),
    m_signalsLayout(new QVBoxLayout(m_container)),
    m_input(new QLineEdit(this)),
    m_btn1(new QPushButton("Client 1", this)),
    m_btn2(new QPushButton("Client 2", this)),
    m_btn3(new QPushButton("Client 3", this)),
    m_btn4(new QPushButton("Client 4", this)),
    m_client1(new Client("127.0.0.1", 3000, this)),
    m_client2(new Client("127.0.0.1", 3001, this)),
    m_client3(new Client("127.0.0.1", 3002, this)),
    m_client4(new Client("127.0.0.1", 3003, this))
{
    setupUI();
}

void DebugWidget::setupUI() {
    auto* controlLay = new QHBoxLayout;
    controlLay->addWidget(m_input);
    controlLay->addWidget(m_btn1);
    controlLay->addWidget(m_btn2);
    controlLay->addWidget(m_btn3);
    controlLay->addWidget(m_btn4);

    auto bindSend = [this](QPushButton* btn, Client* client) {
        connect(btn, &QPushButton::clicked, this, [this,client](){
            QByteArray text = m_input->text().toUtf8();
            client->send(text);
        });
    };
    bindSend(m_btn1, m_client1);
    bindSend(m_btn2, m_client2);
    bindSend(m_btn3, m_client3);
    bindSend(m_btn4, m_client4);

    // 2) Scroll area for messages
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(m_container);

    // 3) Main layout
    auto* topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0,0,0,0);
    topLayout->addLayout(controlLay);
    topLayout->addWidget(m_scrollArea);

    setMinimumWidth(200);
}

static QWidget* makeColoredItem(const QByteArray& data, const QColor& color) {
    auto* frame = new QFrame;
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);
    frame->setAutoFillBackground(true);
    frame->setAttribute(Qt::WA_StyledBackground, true);
    frame->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(color.name()));

    auto* layout = new QHBoxLayout(frame);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(4);

    auto* label = new QLabel(QString::fromUtf8(data), frame);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(label);

    return frame;
}

void DebugWidget::onDataSignalTrigger(QTcpSocket* client,
                                      const QByteArray& data,
                                      const QColor& color,
                                      const QString action)
{
    // Timestamp and peer info
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QHostAddress peer = client->peerAddress();
    quint32 ipv4 = peer.toIPv4Address();
    QString ipString = ipv4 ? QHostAddress(ipv4).toString() : peer.toString();
    quint16 serverPort = client->localPort();

    // Build display string
    QString info = QString("[%1] %2 -> %3:%4 : %5")
                       .arg(timestamp)
                       .arg(action)
                       .arg(ipString)
                       .arg(serverPort)
                       .arg(QString::fromUtf8(data));

    QWidget* item = makeColoredItem(info.toUtf8(), color);
    m_signalsLayout->addWidget(item);
}

void DebugWidget::onSendSignal(QTcpSocket* client, const QByteArray& data) {
    onDataSignalTrigger(client, data, Qt::green,   "Sent");
}

void DebugWidget::onReceiveSignal(QTcpSocket* client, const QByteArray& data) {
    onDataSignalTrigger(client, data, Qt::lightGray, "Received");
}

DebugWidget& DebugWidget::getInstance() {
    static DebugWidget instance;
    return instance;
}

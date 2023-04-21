#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QtGlobal>
#include <QDebug>
#include <QLabel>
#include "crc_verify.h"
#include "udpframe.h"
#include "preferencedialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class fileTransfer; }
QT_END_NAMESPACE

class fileTransfer : public QMainWindow
{
    Q_OBJECT

public:
    QUdpSocket *sendSocket;     //udp发送接口
    QUdpSocket *receiveSocket;   //udp接收接口
    QTimer timerSend;           //发送定时器
    QTimer timerReceive;        //接收定时器
    QString thisip = "127.0.0.1";   //本地ip地址
    int thisUDPPort = 43108;   //本地端口号
    QString sendip = "127.0.0.1";   //目标ip地址
    int sendUDPPort = 42020;   //目标端口号
    UDPFrame* sendUDPFrame;     //发送的UDP帧存储
    UDPFrame* receiveUDPFrame;  //收到的UDP帧存储
    int dataSize = 1024;        //数据字段长度
    int errRate = 10;           //数据出错率：每errRate个包出错一个
    int lostRate = 10;          //数据丢失率：每lostRate个包丢失一个
    int swSize = 4;             //滑动窗口大小
    int initSeqNo = 1;          //起始PDU序号
    int timeOut = 1000;         //超时定时器时长
    QByteArray receiveBuffer;   //文件接收缓冲区
    QLabel* statusLabel = new QLabel();         //状态栏信息

    preferenceDialog *myPrefer = new preferenceDialog(this);

    fileTransfer(QWidget *parent = nullptr);
    ~fileTransfer();

    void on_receiveSocket_readyRead();

private slots:
    void on_actionmenuPrefer_triggered();
    void slotGetParam(QString param);

private:
    Ui::fileTransfer *ui;
};
#endif // FILETRANSFER_H

#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QtGlobal>
#include <QDebug>
#include "crc_verify.h"
#include "udpframe.h"

QT_BEGIN_NAMESPACE
namespace Ui { class fileTransfer; }
QT_END_NAMESPACE

class fileTransfer : public QMainWindow
{
    Q_OBJECT

public:
    QUdpSocket *udpSocket;  //udp传输接口
    QTimer timerSend;       //发送定时器
    QTimer timerReceive;    //接收定时器
    QString ip = "127.0.0.1";   //ip地址
    uint16_t UDPPort = 43108;   //端口号
    UDPFrame* sendUDPFrame;     //发送的UDP帧存储
    UDPFrame* receiveUDPFrame;  //收到的UDP帧存储
    int dataSize = 1024;        //数据字段长度


    fileTransfer(QWidget *parent = nullptr);
    ~fileTransfer();

private:
    Ui::fileTransfer *ui;
};
#endif // FILETRANSFER_H

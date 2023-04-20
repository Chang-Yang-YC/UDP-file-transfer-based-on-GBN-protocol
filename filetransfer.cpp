#include "filetransfer.h"
#include "ui_filetransfer.h"

fileTransfer::fileTransfer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::fileTransfer)
{
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);
    //绑定
    udpSocket->bind(QHostAddress(ip),UDPPort);

    uint8_t check;
    QString test = "123456789";
    QByteArray send = test.toLocal8Bit();
    uint16_t ans = crc16_CCITT(send.data(),9);
    qDebug() << send.size();
    qDebug() << ans;
    qDebug() << QString::number(ans, 16).rightJustified(sizeof(short)*2, '0');


    for(int i = 0;i < send.size();i++)
    {
        check = send[i]; //得到高位
        qDebug() << i + 1 << ": " << QString::number(check, 2).rightJustified(8, '0');
    }
    uint8_t temp;
    temp = ans >> 8;
    qDebug() << QString::number(temp, 2).rightJustified(8, '0');
    send.append(temp);
    temp = ans & 0xff;
    qDebug() << QString::number(temp, 2).rightJustified(8, '0');
    send.append(temp);

//    uint8_t check;
//    int len = send.size();
//    qDebug() << send.toHex();
//    check = send[2]; //得到高位
//    qDebug() << QString::number(check, 2).rightJustified(8, '0');
//    check = check << 8;//向左移8位。结果：0xcd00
//    check = check | send[len - 1];//或上a[1]=0xe2;  即0xcde2
//    qDebug() << QString::number(check, 2).rightJustified(8, '0');
    for(int i = 0;i < send.size();i++)
    {
        check = send[i]; //得到高位
        qDebug() << i + 1 << ": " << QString::number(check, 2).rightJustified(8, '0');
    }
    if(check_CCITT(send.data(),send.size()))   qDebug() << "yes";
}

fileTransfer::~fileTransfer()
{
    delete ui;
}


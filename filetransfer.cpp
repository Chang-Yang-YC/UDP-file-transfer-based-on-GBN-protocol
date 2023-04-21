#include "filetransfer.h"
#include "ui_filetransfer.h"

fileTransfer::fileTransfer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::fileTransfer)
{
    ui->setupUi(this);

    sendSocket = new QUdpSocket(this);
    receiveSocket = new QUdpSocket(this);
    //绑定
    sendSocket->bind(QHostAddress(sendip),sendUDPPort);
    receiveSocket->bind(QHostAddress(thisip),thisUDPPort);


    connect(myPrefer,SIGNAL(send_Back_Param(QString)),this,SLOT(slotGetParam(QString)));


}

fileTransfer::~fileTransfer()
{
    delete ui;
}

void fileTransfer::on_receiveSocket_readyRead()
{
    receiveBuffer.clear();
    //准备ip 端口 存储空间
    QHostAddress ClientAddr; //对方IP存储空间
    uint16_t port;            //对方端口

    //收报文与
    long long len = receiveSocket->readDatagram(receiveBuffer.data(),dataSize + 50,&ClientAddr,&port);

    //判断报文是否读取成功
    if(len < 1) return; //读取失败
    else
    {
        if(check_CCITT(receiveBuffer.data(),receiveBuffer.size()))
        {
            //此时正确性校验不通过
            qDebug() << "crc check failed!";
            return;
        }
        else
        {
            //将接收数据拆解为帧形式
            receiveUDPFrame->getReceive(receiveBuffer.data(),receiveBuffer.size());

        }
    }
}


void fileTransfer::on_actionmenuPrefer_triggered()
{
    QString param;
    param += thisip + "##";
    param += QString("%1##").arg(thisUDPPort);
    param += QString("%1##%2##%3##%4##%5##%6").arg(dataSize).arg(errRate).arg(lostRate).arg(swSize).arg(initSeqNo).arg(timeOut);
    this->myPrefer->show_now_param(param);
    this->myPrefer->setWindowTitle("首选项");
    this->myPrefer->exec();
}

void fileTransfer::slotGetParam(QString param)
{
    qDebug() << param;
    QString shows = "设置参数: " + param;
    statusLabel->setText(shows);
    ui->statusbar->addWidget(statusLabel);
    thisip = param.section("##",0,0);
    thisUDPPort = param.section("##",1,1).toInt();
    dataSize = param.section("##",2,2).toInt();
    errRate = param.section("##",3,3).toInt();
    lostRate = param.section("##",4,4).toInt();
    swSize = param.section("##",5,5).toInt();
    initSeqNo = param.section("##",6,6).toInt();
    timeOut = param.section("##",7,7).toInt();
    receiveSocket->bind(QHostAddress(thisip),thisUDPPort);
}

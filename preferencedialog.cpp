#include "preferencedialog.h"
#include "ui_preferencedialog.h"

//QString thisip;             //本地ip地址
//uint16_t thisUDPPort;       //本地端口号
//int dataSize = 1024;        //数据字段长度
//int errRate = 10;           //数据出错率：每errRate个包出错一个
//int lostRate = 10;          //数据丢失率：每lostRate个包丢失一个
//int swSize = 4;             //滑动窗口大小
//int initSeqNo = 1;          //起始PDU序号
//int timeOut = 1000;         //超时定时器时长

preferenceDialog::preferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::preferenceDialog)
{
    ui->setupUi(this);
}

preferenceDialog::~preferenceDialog()
{
    delete ui;
}

void preferenceDialog::show_now_param(QString msg)
{
    thisip = msg.section("##",0,0);
    thisUDPPort = msg.section("##",1,1);
    dataSize = msg.section("##",2,2).toInt();
    errRate = msg.section("##",3,3).toInt();
    lostRate = msg.section("##",4,4).toInt();
    swSize = msg.section("##",5,5).toInt();
    initSeqNo = msg.section("##",6,6).toInt();
    timeOut = msg.section("##",7,7).toInt();

    displayParam();
}

void preferenceDialog::displayParam()
{
    ui->lineEditIP->setText(thisip);
    ui->lineEditPort->setText(thisUDPPort);
    ui->lineEditDataSize->setText(QString::number(dataSize));
    ui->lineEditErRate->setText(QString::number(errRate));
    ui->lineEditLostRate->setText(QString::number(lostRate));
    ui->lineEditswSize->setText(QString::number(swSize));
    ui->lineEditInitSeqNo->setText(QString::number(initSeqNo));
    ui->lineEditTimeOut->setText(QString::number(timeOut));
}

void preferenceDialog::getParam()
{
    thisip = ui->lineEditIP->text();
    thisUDPPort = ui->lineEditPort->text();
    dataSize = ui->lineEditDataSize->text().toInt();
    errRate = ui->lineEditErRate->text().toInt();
    lostRate = ui->lineEditLostRate->text().toInt();
    swSize = ui->lineEditswSize->text().toInt();
    initSeqNo = ui->lineEditInitSeqNo->text().toInt();
    timeOut = ui->lineEditTimeOut->text().toInt();
}

void preferenceDialog::on_buttonBox_accepted()
{
    getParam();
    QString param;
    param += thisip + "##";
    param += thisUDPPort + "##";
    param += QString("%1##%2##%3##%4##%5##%6").arg(dataSize).arg(errRate).arg(lostRate).arg(swSize).arg(initSeqNo).arg(timeOut);
    emit send_Back_Param(param);
}

void preferenceDialog::on_buttonBox_rejected()
{
}

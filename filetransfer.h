#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimer>
#include <QThread>
#include <QTime>
#include <QHostAddress>
#include <QtGlobal>
#include <QDebug>
#include <QLabel>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QTableWidget>
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
    QTimer timerTimeOut;        //超时定时器
    QString thisip = "127.0.0.1";   //本地ip地址
    quint16 thisUDPPort = 43108;   //本地端口号
    QString sendip = "127.0.0.1";   //目标ip地址
    quint16 sendUDPPort = 42020;   //目标端口号
    UDPFrame* sendUDPFrame;     //发送的UDP帧存储
    UDPFrame* receiveUDPFrame;  //收到的UDP帧存储
    int dataSize = 1024;        //数据字段长度
    int errRate = 50;           //数据出错率：每errRate个包出错一个
    int lostRate = 50;          //数据丢失率：每lostRate个包丢失一个
    int swSize = 4;             //滑动窗口大小
    int initSeqNo = 1;          //起始PDU序号
    int timeOut = 1000;         //超时定时器时长
    QByteArray receiveBuffer;   //文件接收缓冲区
    QByteArray loadBuf;         //装载进缓冲区的媒介
    char** sendBuf;             //发送缓冲区
    long long* buftoNo;         //sendBuf中对应的帧序号
    int* buflen;                //sendBuf对应长度
    bool isSending = 0;         //记录此刻是不是在发文件
    bool isReceiveing = 0;      //记录此刻是不是在收文件
    int noAnsCount;             //记录连续有多少起无回应，超过十次认为信道失效
    int timeOutCount;           //记录连续多少次重发，连续8次则认为信道失效

    long long frameNo  = -1;    //当前发送帧序号
    long long ackfraNo = -1;    //已确认的帧号    !!!发送者维护
    long long recefraNo = -1;   //当前接收帧序号  !!!接收者维护
    long long TimerackNo = -2;       // ！！！超时计时器维护  记录上次确认的帧
    long long TimeOutTotall = 0;
    QTime* startTime;
    int runtime = 0;
    float sendSpeed;
    float maxSpeed = 0;

    //发送文件相关
    QFile file;       //文件对象
    QString fileName; //文件名字
    long long fileSize; //文件大小
    long long sendSize; //已经发送文件的大小
    long long expectNo = -1;         //发送完文件需要多少帧

    //接收文件相关
    QFile receiveFile;
    QString receiveFileName;
    QString receivePath;
    long long receiveFileSize;
    long long receivedSize;
    long long expectReceive = -1;         //发送完文件需要多少帧

    //日志保存相关
    QTableWidget* logTable;
    int rowlog = 0;
    QFile logFile;
    QString logName;
    QString logPath;

    QTime* myTime;

    //table1相关
    QTableWidget* table1;
    int row = 0;
    int ackrow = 0;

    //table2相关
    QTableWidget* table2;
    int rowReceive = 0;

    //tableItem
    //类型
    QTableWidgetItem* tableItemTypeACK;
    QTableWidgetItem* tableItemTypeData;
    QTableWidgetItem* tableItemTypeBoth;
    //校验
    QTableWidgetItem* tableItemCheckOK;
    QTableWidgetItem* tableItemCheckDataErr;
    QTableWidgetItem* tableItemCheckNoErr;
    //状态
    QTableWidgetItem* tableItemSending;
    QTableWidgetItem* tableItemACK;
    QTableWidgetItem* tableItemUnaccept;



    QByteArray debugarray;
    QLabel* statusLabel = new QLabel();         //状态栏信息

    preferenceDialog *myPrefer = new preferenceDialog(this);

    fileTransfer(QWidget *parent = nullptr);
    ~fileTransfer();

    void on_receiveSocket_readyRead();
    void timerSend_triggered();
    void analyseReceive();
    void timeOutCheck();

    bool isFrameMistake();  //人为引入帧出错
    bool isFrameLost();     //人为引入帧丢失

private slots:
    void on_actionmenuPrefer_triggered();
    void slotGetParam(QString param);

    void on_pushButton1OpenFile_clicked();

    void on_pushButton1SaveLog_clicked();

    void on_pushButton1Send_clicked();

    void on_pushButton2SaveLog_clicked();

    void on_pushButton3LoadLog_clicked();

    void on_actionLoadParam_triggered();

    void on_pushButton_clicked();

private:
    Ui::fileTransfer *ui;
};
#endif // FILETRANSFER_H

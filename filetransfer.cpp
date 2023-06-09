#include "filetransfer.h"
#include "ui_filetransfer.h"

fileTransfer::fileTransfer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::fileTransfer)
{
    ui->setupUi(this);

    startTime = new QTime();
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));  //随机种子
    sendSocket = new QUdpSocket(this);
    receiveSocket = new QUdpSocket(this);
    //绑定接收端口
    receiveSocket->bind(QHostAddress(thisip),thisUDPPort);
    ui->pushButton1Send->setEnabled(false);
    ui->pushButton1SaveLog->setEnabled(false);

    sendUDPFrame = new UDPFrame();
    receiveUDPFrame = new UDPFrame();

    myTime = new QTime();
    //初始化表格1
    tableItemSending = new QTableWidgetItem();
    tableItemSending->setText("已发送");
    tableItemSending->setBackgroundColor(Qt::yellow);
    tableItemACK = new QTableWidgetItem();
    tableItemACK->setText("已确认");
    tableItemACK->setBackgroundColor(Qt::green);
    tableItemUnaccept = new QTableWidgetItem();
    tableItemUnaccept->setText("未接受");
    tableItemUnaccept->setBackgroundColor(Qt::red);
    tableItemTypeACK = new QTableWidgetItem();
    tableItemTypeACK->setText("无数据帧");
    tableItemTypeACK->setBackgroundColor(Qt::gray);
    table1 = ui->tableWidget1;
    table1->scrollToBottom();
    table1->setColumnCount(7);
    table1->setRowCount(1);
    QStringList table1List;
    table1List << tr("帧序号") << tr("时间戳") << tr("帧类型") << tr("状态") << tr("确认帧") << tr("帧校验") << tr("已发送");
    table1->setHorizontalHeaderLabels(table1List);
    table1->setEditTriggers(QAbstractItemView::NoEditTriggers);   //表格不可编辑

    //初始化表格2
    tableItemTypeBoth = new QTableWidgetItem();
    tableItemTypeBoth->setText("复合帧");
    tableItemTypeBoth->setBackgroundColor(Qt::cyan);
    tableItemTypeData = new QTableWidgetItem();
    tableItemTypeData->setText("数据帧");
    tableItemTypeData->setBackgroundColor(Qt::magenta);
    tableItemCheckOK = new QTableWidgetItem();
    tableItemCheckOK->setText("有效帧");
    tableItemCheckOK->setBackgroundColor(Qt::green);
    tableItemCheckDataErr = new QTableWidgetItem();
    tableItemCheckDataErr->setText("数据错误");
    tableItemCheckDataErr->setBackgroundColor(Qt::red);
    tableItemCheckNoErr = new QTableWidgetItem();
    tableItemCheckNoErr->setText("帧序错误");
    tableItemCheckNoErr->setBackgroundColor(Qt::yellow);

    table2 = ui->tableWidget2;
    table2->setColumnCount(6);
    table2->setRowCount(1);
    QStringList table2List;
    table2List << tr("帧序号") << tr("时间戳") << tr("帧类型") << tr("校验") << tr("ack帧") << tr("已接收");
    table2->setHorizontalHeaderLabels(table2List);
    table2->setEditTriggers(QAbstractItemView::NoEditTriggers);   //表格不可编辑

    logTable = ui->tableWidget3;
    logTable->setColumnCount(9);
    logTable->setRowCount(1);
    QStringList table3List;
    table3List << tr("文件大小") << tr("窗口大小") << tr("错误率") << tr("丢失率") << tr("超时时长/ms") << tr("UDP帧数")
               << tr("超时次数") << tr("发送时长/ms") << tr("平均速率/kb-s");
    logTable->setHorizontalHeaderLabels(table3List);
    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(myPrefer,SIGNAL(send_Back_Param(QString)),this,SLOT(slotGetParam(QString)));
    connect(&timerSend,&QTimer::timeout,[=](){timerSend_triggered();});
    connect(&timerTimeOut,&QTimer::timeout,[=](){timeOutCheck();});
    connect(receiveSocket,&QUdpSocket::readyRead,this,&fileTransfer::on_receiveSocket_readyRead);

}

fileTransfer::~fileTransfer()
{
    delete ui;
}

void fileTransfer::on_receiveSocket_readyRead()
{
    receiveBuffer.clear();
    receiveBuffer.resize(dataSize + 50);
    while(receiveSocket->hasPendingDatagrams())
    {
    receiveBuffer.clear();
    receiveBuffer.resize(receiveSocket->pendingDatagramSize());

    //准备ip 端口 存储空间
    QHostAddress ClientAddr; //对方IP存储空间
    uint16_t port;            //对方端口
    receiveSocket->readDatagram(receiveBuffer.data(),receiveBuffer.size(),&ClientAddr,&port);
    //收报文
    //判断报文是否读取成功
    if(receiveBuffer.size() < 1) return; //读取失败
    else
    {
        if(check_CCITT(receiveBuffer.data(),receiveBuffer.size()))
        {
            //此时正确性校验不通过
            //qDebug() << "crc check failed!";
            table2->setItem(rowReceive,0,new QTableWidgetItem("NULL")); //帧序号
            table2->setItem(rowReceive,1,new QTableWidgetItem(myTime->currentTime().toString("hh:mm:ss.zzz"))); //时间戳
            table2->setItem(rowReceive,2,new QTableWidgetItem("NULL")); //类型
            table2->setItem(rowReceive,3,tableItemCheckDataErr->clone()); //校验
            table2->setItem(rowReceive,4,new QTableWidgetItem("NULL")); //ack
            table2->setItem(rowReceive,5,new QTableWidgetItem("NULL")); //接收量
            rowReceive++;
            table2->insertRow(rowReceive);
            table2->scrollToBottom();
            return;
        }
        else
        {
            //qDebug() << "crc check pass.";
            //将接收数据拆解为帧形式
            sendip = ClientAddr.toString();
            sendUDPPort = (int)port;
            receiveUDPFrame->getReceive(receiveBuffer);
            analyseReceive();
        }
    }
    }
}

void fileTransfer::analyseReceive()
{

    if(isReceiveing)
    {
        statusLabel->setText(QString("receiveing frame: %1").arg(receiveUDPFrame->frame_expected));
        ui->statusbar->addWidget(statusLabel);
    }
    if(receiveUDPFrame->frame_expected == 0)    //收到文件头消息，本地身份新增接收者
    {
        QString fileInfo(receiveUDPFrame->arrSend);
        receiveFileName = fileInfo.section("##",0,0);
        receiveFileSize = fileInfo.section("##",1,1).toLongLong();
        expectReceive = fileInfo.section("##",2,2).toLongLong();
        //关联文件名字

        receivePath = "./port" + QString::number(thisUDPPort) + "receive";
        QDir save;
        if(!save.exists(receivePath))   save.mkdir(receivePath);
        receivePath += "/" + receiveFileName;

        receiveFile.setFileName(receivePath);

        //只写方式方式，打开文件
        bool isOk = receiveFile.open(QIODevice::WriteOnly);
        if(isOk == true)
        {
            //qDebug() << "open file successful";
            isReceiveing = true;
            receivedSize = 0;
            recefraNo = 0;
            if(timerSend.isActive() == false)
            {
                sendUDPFrame->init();
                sendUDPFrame->setHead(1,-1,0);
                sendUDPFrame->framing();
                sendSocket->writeDatagram(sendUDPFrame->arrSend.data(),sendUDPFrame->totalLen,QHostAddress(sendip),sendUDPPort);
                timerSend.start(20);
            }
        }
        else{
            statusLabel->setText("文件建立失败");
            ui->statusbar->addWidget(statusLabel);
        }
    }
    else if(isReceiveing == true)   //自己是接收者，需要接收文件帧信息和数据
    {
        if(recefraNo + 1 == receiveUDPFrame->frame_expected) //接收端单窗口
        {
            int writelen = receiveFile.write(receiveUDPFrame->arrSend,receiveUDPFrame->totalLen);
            if(writelen > 0)
            {
                recefraNo++;
                receivedSize += writelen;
            }
        }
        if(receivedSize == receiveFileSize)  //接收完成
        {
            receiveFile.close(); //关闭文件
            //断开连接
            receiveSocket->disconnectFromHost();
            receiveSocket->bind(QHostAddress(thisip),thisUDPPort);
            isReceiveing = false;

            statusLabel->setText("文件接收完成");
            ui->statusbar->addWidget(statusLabel);
        }
    }
    if(isSending == true && ackfraNo < receiveUDPFrame->ack_expected)       //自己是发送者，需要接收ack消息
    {
        for(int i = ackfraNo;i < receiveUDPFrame->ack_expected;i++)
        {
            table1->setItem(ackrow,3,tableItemACK->clone()); //状态

            ackrow++;
        }
        ackfraNo = receiveUDPFrame->ack_expected;
    }

    if(isReceiveing && isSending)  //在接受数据
    {
        table2->setItem(rowReceive,1,new QTableWidgetItem(myTime->currentTime().toString("hh:mm:ss.zzz"))); //时间戳
        table2->setItem(rowReceive,0,new QTableWidgetItem(QString::number(receiveUDPFrame->frame_expected))); //帧序号
        table2->setItem(rowReceive,2,tableItemTypeBoth->clone());
        if(receiveUDPFrame->frame_expected == recefraNo)    table2->setItem(rowReceive,3,tableItemCheckOK->clone());
        else    table2->setItem(rowReceive,3,tableItemCheckNoErr->clone());
        table2->setItem(rowReceive,4,new QTableWidgetItem(QString::number(receiveUDPFrame->ack_expected))); //ack
        table2->setItem(rowReceive,5,new QTableWidgetItem(QString("%1/%2").arg(receivedSize).arg(receiveFileSize))); //已发送大小
    }
    else if(isReceiveing)
    {
        table2->setItem(rowReceive,1,new QTableWidgetItem(myTime->currentTime().toString("hh:mm:ss.zzz"))); //时间戳
        table2->setItem(rowReceive,0,new QTableWidgetItem(QString::number(receiveUDPFrame->frame_expected))); //帧序号
        table2->setItem(rowReceive,2,tableItemTypeData->clone());
        if(receiveUDPFrame->frame_expected == recefraNo)    table2->setItem(rowReceive,3,tableItemCheckOK->clone());
        else    table2->setItem(rowReceive,3,tableItemCheckNoErr->clone());
        table2->setItem(rowReceive,4,new QTableWidgetItem("NULL")); //ack
        table2->setItem(rowReceive,5,new QTableWidgetItem(QString("%1/%2").arg(receivedSize).arg(receiveFileSize))); //已发送大小
    }
    else if(isSending)
    {
        table2->setItem(rowReceive,1,new QTableWidgetItem(myTime->currentTime().toString("hh:mm:ss.zzz"))); //时间戳
        table2->setItem(rowReceive,0,new QTableWidgetItem("NULL")); //帧序号
        table2->setItem(rowReceive,2,tableItemTypeACK->clone());
        table2->setItem(rowReceive,3,tableItemCheckOK->clone());
        table2->setItem(rowReceive,4,new QTableWidgetItem(QString::number(receiveUDPFrame->ack_expected))); //ack
        table2->setItem(rowReceive,5,new QTableWidgetItem("NULL")); //已发送大小
    }
    rowReceive++;
    table2->insertRow(rowReceive);
    table2->scrollToBottom();
}

void fileTransfer::on_pushButton1Send_clicked()
{
    //先传文件信息，然后激活定时器传文件。
    frameNo = 1;
    ackfraNo = -1;

    QString head = fileName + "##" + QString::number(fileSize,10)+"##"+QString::number(expectNo,10);
    sendip = ui->lineEditIP->text();
    sendUDPPort = ui->lineEditPort->text().toUShort();
    sendUDPFrame->setHead(0,0,0);
    sendUDPFrame->setBuffer(head.toLocal8Bit());
    sendUDPFrame->framing();
    sendSocket->writeDatagram(sendUDPFrame->arrSend.data(),sendUDPFrame->totalLen,QHostAddress(sendip),sendUDPPort);

    startTime->start();
    QTime delay = QTime::currentTime().addMSecs(20);
    while(QTime::currentTime()<delay)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents,5);
        QThread::msleep(5);
    }
    isSending = true;

    table1->setItem(row,0,new QTableWidgetItem("0")); //帧序号
    table1->setItem(row,1,new QTableWidgetItem(myTime->currentTime().toString("hh:mm:ss.zzz"))); //时间戳
    if(isSending && isReceiveing)
    {
        table1->setItem(row,2,tableItemTypeBoth->clone()); //类型
        table1->setItem(row,4,new QTableWidgetItem(QString::number(sendUDPFrame->ack_expected))); //ack帧
    }
    else if(isSending)
    {
        table1->setItem(row,2,tableItemTypeData->clone()); //类型
        table1->setItem(row,4,new QTableWidgetItem("NULL")); //ack帧
    }
    else if(isReceiveing)
    {
        table1->setItem(row,2,tableItemTypeACK->clone()); //类型
        table1->setItem(row,4,new QTableWidgetItem(QString::number(sendUDPFrame->ack_expected))); //ack帧
    }
    table1->setItem(row,3,tableItemSending->clone()); //状态
    table1->setItem(row,5,new QTableWidgetItem(QString::number(sendUDPFrame->crc,16))); //crc校验
    table1->setItem(row,6,new QTableWidgetItem(QString("%1/%2").arg(sendSize).arg(fileSize))); //已发送大小
    row++;
    ui->tableWidget1->insertRow(row);
    //激活超时计时器和发送定时器
    if(timerTimeOut.isActive() == false)    timerTimeOut.start(timeOut);
    if(timerSend.isActive() == false)       timerSend.start(20);

    sendBuf = new char*[swSize];
    for(int i = 0;i < swSize;i++)
    {
        sendBuf[i] = new char[dataSize];
    }
    buftoNo = (long long*)malloc(swSize*sizeof(long long));
    buflen = (int*)malloc(swSize*sizeof(int));
    loadBuf.resize(dataSize);
    sendSize = 0;
}

void fileTransfer::timerSend_triggered()
{
    sendip = ui->lineEditIP->text();
    sendUDPPort = ui->lineEditPort->text().toUShort();
    //先装载头部
    sendUDPFrame->init();
    sendUDPFrame->setHead(-1,-1,-1);    //int fnex, int fexp, int fack
    if(isReceiveing == true)  //当前是接收者，需要返回ack消息
    {
        sendUDPFrame->ack_expected = recefraNo;
        sendUDPFrame->next_frame_to_send = recefraNo + 1;
    }
    if(isSending == true && fileSize == sendSize)
    {
        runtime += startTime->restart();
        statusLabel->setText(QString("发送时长: %1s").arg(runtime / 1000));
        if(isSending)   ui->statusbar->addWidget(statusLabel);
        ui->pushButton1SaveLog->setEnabled(true);
        isSending = false;
        while(ackrow < row)
        {
            table1->setItem(ackrow,3,tableItemACK->clone()); //状态
            ackrow++;
        }
    }
                    //当前是发送者，需要发送帧序号和数据
    else if(isSending == true && frameNo - ackfraNo <= swSize && fileSize > sendSize)    //在窗口内，装载数据
    {
        noAnsCount = 0;
        sendUDPFrame->frame_expected = frameNo;
        if(buftoNo[frameNo % swSize] != frameNo)    //操作发送方的窗口
        {
            buftoNo[frameNo % swSize] = frameNo;
            buflen[frameNo % swSize] = file.read(sendBuf[frameNo % swSize],dataSize);
            //buflen[frameNo % swSize] = file.read(loadBuf.data(),dataSize);
            sendSize += buflen[frameNo % swSize];
        }

        //装载完数据进帧
        loadBuf.clear();
        loadBuf.resize(buflen[frameNo % swSize]);
        for(int i = 0;i < buflen[frameNo % swSize];i++) loadBuf[i] = sendBuf[frameNo % swSize][i];
        sendUDPFrame->setBuffer(loadBuf);
        //sendUDPFrame->setBuffer(sendBuf[frameNo % swSize],buflen[frameNo % swSize]);
        frameNo++;
    }
    else if(isSending == true && fileSize > sendSize)
    {
        noAnsCount++;
        //sendUDPFrame->frame_expected = frameNo;
        if(noAnsCount > 100) isSending = false;  //超过50次，认为信道不可用
    }


    //发送数据
    sendUDPFrame->framing();

    if(isSending && isReceiveing)
    {
        table1->setItem(row,1,new QTableWidgetItem(myTime->currentTime().toString("hh:mm:ss.zzz"))); //时间戳
        table1->setItem(row,3,tableItemSending->clone()); //状态
        table1->setItem(row,5,new QTableWidgetItem(QString::number(sendUDPFrame->crc,16))); //crc校验
        table1->setItem(row,0,new QTableWidgetItem(QString::number(sendUDPFrame->frame_expected))); //帧序号
        table1->setItem(row,2,tableItemTypeBoth->clone()); //类型
        table1->setItem(row,4,new QTableWidgetItem(QString::number(sendUDPFrame->ack_expected))); //ack帧
        table1->setItem(row,6,new QTableWidgetItem(QString("%1/%2").arg(sendSize).arg(fileSize))); //已发送大小
    }
    else if(isSending)
    {
        table1->setItem(row,1,new QTableWidgetItem(myTime->currentTime().toString("hh:mm:ss.zzz"))); //时间戳
        table1->setItem(row,3,tableItemSending->clone()); //状态
        table1->setItem(row,5,new QTableWidgetItem(QString::number(sendUDPFrame->crc,16))); //crc校验
        if(sendUDPFrame->frame_expected == -1)
        {
            table1->setItem(row,0,new QTableWidgetItem("NULL")); //帧序号
            table1->setItem(row,2,tableItemTypeACK->clone()); //类型
        }
        else
        {
            table1->setItem(row,0,new QTableWidgetItem(QString::number(sendUDPFrame->frame_expected))); //帧序号
            table1->setItem(row,2,tableItemTypeData->clone()); //类型
        }
        table1->setItem(row,4,new QTableWidgetItem("NULL")); //ack帧
        table1->setItem(row,6,new QTableWidgetItem(QString("%1/%2").arg(sendSize).arg(fileSize))); //已发送大小
    }
    else if(isReceiveing)
    {
        table1->setItem(row,1,new QTableWidgetItem(myTime->currentTime().toString("hh:mm:ss.zzz"))); //时间戳
        table1->setItem(row,3,tableItemSending->clone()); //状态
        table1->setItem(row,5,new QTableWidgetItem(QString::number(sendUDPFrame->crc,16))); //crc校验
        table1->setItem(row,0,new QTableWidgetItem("NULL")); //帧序号
        table1->setItem(row,2,tableItemTypeACK->clone()); //类型
        table1->setItem(row,4,new QTableWidgetItem(QString::number(sendUDPFrame->ack_expected))); //ack帧
        table1->setItem(row,6,new QTableWidgetItem("NULL")); //
    }
    row++;
    table1->insertRow(row);
    table1->scrollToBottom();
    runtime += startTime->restart();
    sendSpeed = 1.024 * sendSize / runtime;
    statusLabel->setText(QString("平均发送速率: %1kb/s").arg(sendSpeed));
    if(isSending)   ui->statusbar->addWidget(statusLabel);
    if(sendSpeed > maxSpeed)    maxSpeed = sendSpeed;

    if(isFrameMistake())    sendUDPFrame->totalLen = 10;

    sendSocket->writeDatagram(sendUDPFrame->arrSend.data(),sendUDPFrame->totalLen,QHostAddress(sendip),sendUDPPort);

    //检查下需不需要关闭发送
    if(!isSending)  timerTimeOut.stop();
    if(!isSending && !isReceiveing) timerSend.stop();
}

//超时计时器，作用为记录在一个超时周期里，最早发出的未确认帧是否收到了ack，如果没收到，则调整发送帧为这个未确认帧,连续八次调整则停止发送
void fileTransfer::timeOutCheck()
{
    //若==，则在一个周期里未收到ack，调整
    if(TimerackNo == ackfraNo)
    {
        while(ackrow < row)
        {
            table1->setItem(ackrow,3,tableItemUnaccept->clone()); //状态
            ackrow++;
        }
        timeOutCount++;
        TimeOutTotall++;
        frameNo = ackfraNo + 1;
        if(timeOutCount > 8)
        {
            isSending = false;
            isReceiveing = false;
            timerSend.stop();
            timerTimeOut.stop();
        }
    }
    else
    {
        timeOutCount = 0;
        TimerackNo = ackfraNo;
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
    thisUDPPort = param.section("##",1,1).toUShort();
    dataSize = param.section("##",2,2).toInt();
    errRate = param.section("##",3,3).toInt();
    lostRate = param.section("##",4,4).toInt();
    swSize = param.section("##",5,5).toInt();
    initSeqNo = param.section("##",6,6).toInt();
    timeOut = param.section("##",7,7).toInt();
    //receiveSocket->disconnectFromHost();
    receiveSocket->bind(QHostAddress(thisip),thisUDPPort);
}


void fileTransfer::on_pushButton1OpenFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "open", "../");
    if(false == filePath.isEmpty()) //如果选择文件路径有效
    {
        fileName.clear();
        fileSize = 0;

        //获取文件信息
        QFileInfo info(filePath);
        fileName = info.fileName(); //获取文件名字
        fileSize = info.size(); //获取文件大小
        expectNo = fileSize / dataSize + 1;
        sendSize = 0;           //已发送文件的大小

        //只读方式打开文件
        //指定文件的名字
        file.setFileName(filePath);

        //打开文件
        bool isOk = file.open(QIODevice::ReadOnly);
        if(false == isOk)
        {
            statusLabel->setText("打开文件失败");
            ui->statusbar->addWidget(statusLabel);
        }
        //提示打开文件的路径
        statusLabel->setText("打开文件成功");
        ui->statusbar->addWidget(statusLabel);
        ui->lineEditFilepath->setText(filePath);
        ui->pushButton1Send->setEnabled(true);
    }
    else
    {
        statusLabel->setText("无效路径");
        ui->statusbar->addWidget(statusLabel);
    }
}

void fileTransfer::on_pushButton1SaveLog_clicked()
{
    logPath = "./LOG";
    QDir save;
    if(!save.exists(logPath))   save.mkdir(logPath);
    logPath += "/SEND-" + fileName + myTime->currentTime().toString("-hh-mm-ss") + ".txt";
    logFile.setFileName(logPath);
    if(!logFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        statusLabel->setText("日志保存失败");
        ui->statusbar->addWidget(statusLabel);
        return;
    }
    QTextStream wrin(&logFile);
    wrin << "filesize##" << fileSize;
    wrin << "##swsize##" << swSize;
    wrin << "##errRate##" << errRate;
    wrin << "##lostRate##" << lostRate;
    wrin << "##timeOut##" << timeOut;
    wrin << "##$$" << endl;

    wrin << "UDPnum##" << row;
    wrin << "##timeoutCount##" << TimeOutTotall;
    wrin << "##runtime##" << runtime;
    wrin << "##speed##" << sendSpeed;
    wrin << "##$$" << endl;

    for(int i = 0;i < row;i++)
    {
        wrin << table1->item(i,0) << "##";
        wrin << table1->item(i,1) << "##";
        wrin << table1->item(i,2) << "##";
        wrin << table1->item(i,3) << "##";
        wrin << table1->item(i,4) << "##";
        wrin << table1->item(i,5) << "##";
        wrin << table1->item(i,6) << "##$$" << endl;
    }
    logFile.close();

}


void fileTransfer::on_pushButton3LoadLog_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "open", "./");
    if(false == filePath.isEmpty()) //如果选择文件路径有效
    {
        //logPath.clear();
        fileSize = 0;

        //只读方式打开文件
        //指定文件的名字
        logFile.setFileName(filePath);

        //打开文件
        bool isOk = logFile.open(QIODevice::ReadOnly);
        if(false == isOk)
        {
            statusLabel->setText("打开日志失败");
            ui->statusbar->addWidget(statusLabel);
        }
        //提示打开文件的路径
        statusLabel->setText("打开日志成功");
        ui->statusbar->addWidget(statusLabel);
        QTextStream read(&logFile);
        QString line = read.readLine();
        logTable->setItem(rowlog,0,new QTableWidgetItem(line.section("##",1,1)));
        logTable->setItem(rowlog,1,new QTableWidgetItem(line.section("##",3,3)));
        logTable->setItem(rowlog,2,new QTableWidgetItem(line.section("##",5,5)));
        logTable->setItem(rowlog,3,new QTableWidgetItem(line.section("##",7,7)));
        logTable->setItem(rowlog,4,new QTableWidgetItem(line.section("##",9,9)));
        line = read.readLine();
        logTable->setItem(rowlog,5,new QTableWidgetItem(line.section("##",1,1)));
        logTable->setItem(rowlog,6,new QTableWidgetItem(line.section("##",3,3)));
        logTable->setItem(rowlog,7,new QTableWidgetItem(line.section("##",5,5)));
        logTable->setItem(rowlog,8,new QTableWidgetItem(line.section("##",7,7)));
        logFile.close();
        rowlog++;
        logTable->insertRow(rowlog);
    }
    else
    {
        statusLabel->setText("无效路径");
        ui->statusbar->addWidget(statusLabel);
    }

}

void fileTransfer::on_actionLoadParam_triggered()
{
    QFile param;
    QString data;
    QString filePath = QFileDialog::getOpenFileName(this, "open", "../");
    if(false == filePath.isEmpty()) //如果选择文件路径有效
    {
        //只读方式打开文件
        //指定文件的名字
        param.setFileName(filePath);

        //打开文件
        bool isOk = param.open(QIODevice::ReadOnly);
        if(false == isOk)
        {
            statusLabel->setText("只读方式打开文件失败");
            ui->statusbar->addWidget(statusLabel);
        }
        //提示打开文件的路径
        statusLabel->setText("打开文件成功");
        ui->statusbar->addWidget(statusLabel);
        while(!param.atEnd())
        {
            QByteArray line = param.readLine();
            data += QString(line);
        }
        data = data.section("$$",1,1);
        QString shows = "设置参数成功";
        statusLabel->setText(shows);
        ui->statusbar->addWidget(statusLabel);
        thisip = data.section("##",0,0);
        thisUDPPort = data.section("##",1,1).toInt();
        dataSize = data.section("##",2,2).toInt();
        errRate = data.section("##",3,3).toInt();
        lostRate = data.section("##",4,4).toInt();
        swSize = data.section("##",5,5).toInt();
        initSeqNo = data.section("##",6,6).toInt();
        timeOut = data.section("##",7,7).toInt();
        sendip = data.section("##",8,8);
        ui->lineEditIP->setText(sendip);
        sendUDPPort = data.section("##",9,9).toUShort();
        ui->lineEditPort->setText(QString::number(sendUDPPort));
        receiveSocket->bind(QHostAddress(thisip),thisUDPPort);
    }
    else
    {
        statusLabel->setText("无效路径");
        ui->statusbar->addWidget(statusLabel);
    }
}

void fileTransfer::on_pushButton_clicked()
{
    isSending = false;
    isReceiveing = false;
    timerSend.stop();
    timerTimeOut.stop();
}

bool fileTransfer::isFrameMistake()
{
    if(errRate == 0)    return false;
    int check = qrand() % errRate;
    return check == 0;
}

bool fileTransfer::isFrameLost()
{
    if(lostRate == 0)    return false;
    int check = qrand() % lostRate;
    return check == 0;
}

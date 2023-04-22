#include "udpframe.h"

//int next_frame_to_send; //响应帧号，接收方发出，发送方核对
//int frame_expected;     //预期帧号，发送方发出，接收方核对
//int ack_expected;       //确认帧号，返回此时最近确认的帧(捎带)
//unsigned char* buffer;  //帧数据字段
//int nbuffered;          //帧数据字段所占空间
//QByteArray arrSend;     //最终发送的

UDPFrame::UDPFrame()
{
    next_frame_to_send = 0;
    frame_expected = 0;
    ack_expected = 0;
}

//UDPFrame::UDPFrame(char *data, int len)
//{
//    arrSend.clear();
//    buffer = data + 25;
//    nbuffered = len - 27;

//    QByteArray head = QByteArray(data, 25);
//    next_frame_to_send = QString(head).section("##", 0, 0).toInt();
//    frame_expected = QString(head).section("##", 1, 1).toInt();
//    ack_expected = QString(head).section("##", 2, 2).toInt();
//    for(int i = 25;i < len - 2;i++)
//    {
//        arrSend.append(data[i]);
//    }
//}

void UDPFrame::framing()
{
    arrSend.clear();
    arrSend.resize(buffer.size()+50);
    QString head = QString("%1##%2##%3##").arg(next_frame_to_send).arg(frame_expected).arg(ack_expected);
    arrSend = head.toLocal8Bit();
    for(int i = arrSend.size();i < 25;i++)   arrSend.append('$');
    //for(int i = 0;i < nbuffered;i++)    arrSend.append(buffer[i]);
    arrSend.append(buffer);
    uint8_t temp;
    uint16_t crc = crc16_CCITT(arrSend.data(),arrSend.size());
    temp = crc >> 8;
    arrSend.append(temp);
    temp = crc & 0xff;
    arrSend.append(temp);
    totalLen = arrSend.size();
}

void UDPFrame::getReceive(QByteArray arr)
{
    //QByteArray head = QByteArray(data, 25);
    buffer.clear();
    arrSend.clear();
    QByteArray head;
    for(int i = 0;i < 25;i++)
    {
        head[i] = arr[i];
    }
    next_frame_to_send = QString(head).section("##", 0, 0).toInt();
    frame_expected = QString(head).section("##", 1, 1).toInt();
    ack_expected = QString(head).section("##", 2, 2).toInt();
    totalLen = arr.size() - 27;
    for(int i = 25;i < arr.size() - 2;i++)
    {
        buffer[i-25] = arr[i];
        arrSend[i - 25] = arr[i];
    }
}

//void UDPFrame::getReceive(char *data, int len)
//{
//    arrSend.clear();
//    buffer.clear();
//    for(int i = 0;i < )
//    buffer = data;
//    nbuffered = len;
//    QByteArray head = QByteArray(data, 25);
//    next_frame_to_send = QString(head).section("##", 0, 0).toInt();
//    frame_expected = QString(head).section("##", 1, 1).toInt();
//    ack_expected = QString(head).section("##", 2, 2).toInt();
//    for(int i = 25;i < len - 2;i++)    arrSend.append(data[i]);
//}

void UDPFrame::setHead(int fnex, int fexp, int fack)
{
    next_frame_to_send = fnex;
    frame_expected = fexp;
    ack_expected = fack;
}

void UDPFrame::setBuffer(QByteArray arr)
{
    buffer.resize(arr.size());
    for(int i = 0;i <arr.size();i++)
    {
        buffer[i] = arr[i];
    }
}

//void UDPFrame::setBuffer(char *data, int len)
//{
//    for(int i = 0;i < len;i++)
//    {
//        buffer[i] = data[i];
//    }
//}

void UDPFrame::init()
{
    setHead(0,0,0);
    buffer.clear();
}

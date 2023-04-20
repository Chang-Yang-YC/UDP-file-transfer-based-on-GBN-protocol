#include "udpframe.h"

//int next_frame_to_send; //响应帧号，接收方发出，发送方核对
//int frame_expected;     //预期帧号，发送方发出，接收方核对
//int ack_expected;       //确认帧号，返回此时最近确认的帧(捎带)
//unsigned char* buffer;  //帧数据字段
//int nbuffered;          //帧数据字段所占空间
//QByteArray arrSend;     //最终发送的

UDPFrame::UDPFrame()
{

}

UDPFrame::UDPFrame(char *data, int len)
{
    arrSend.clear();
    QByteArray head = QByteArray(data, 25);
    next_frame_to_send = QString(head).section("##", 0, 0).toInt();
    frame_expected = QString(head).section("##", 1, 1).toInt();
    ack_expected = QString(head).section("##", 2, 2).toInt();
    for(int i = 25;i < len - 2;i++)    arrSend.append(data[i]);
}

void UDPFrame::framing()
{
    arrSend.clear();
    QString head = QString("%1##%2##%3##").arg(next_frame_to_send).arg(frame_expected).arg(ack_expected);
    arrSend = head.toLocal8Bit();
    for(int i = 0;i < 25;i++)   arrSend.append('$');
    for(int i = 0;i < nbuffered;i++)    arrSend.append(buffer[i]);
    uint8_t temp;
    uint16_t crc = crc16_CCITT(arrSend.data(),arrSend.size());
    temp = crc >> 8;
    arrSend.append(temp);
    temp = crc & 0xff;
    arrSend.append(temp);
}

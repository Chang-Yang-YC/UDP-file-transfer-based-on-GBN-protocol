#ifndef UDPFRAME_H
#define UDPFRAME_H

#include <iostream>
#include <QObject>
#include "crc_verify.h"

class UDPFrame
{
public:
    int next_frame_to_send; //响应帧号，接收方发出，发送方核对
    int frame_expected;     //预期帧号，发送方发出，接收方核对
    int ack_expected;       //确认帧号，返回此时最近确认的帧(捎带)
    unsigned char* buffer;  //帧数据字段
    int nbuffered;          //帧数据字段所占空间
    QByteArray arrSend;     //最终发送的

    UDPFrame();
    UDPFrame(char* data, int len);  //接收者根据字符串组帧

    void framing();
};

#endif // UDPFRAME_H

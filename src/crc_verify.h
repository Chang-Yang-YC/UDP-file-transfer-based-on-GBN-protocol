#ifndef CRC_VERIFY_H
#define CRC_VERIFY_H

#include <QObject>
#include <QDebug>
#include "stdint.h"
#include "stdio.h"

uint16_t crc16_CCITT(uint8_t* datas,uint16_t len);
uint16_t crc16_CCITT(char* data,uint16_t len);
bool check_CCITT(char* data, uint16_t len);

#endif // CRC_VERIFY_H

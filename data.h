#ifndef DATAH_H
#define DATAH_H

#include <QString>
#include <stdio.h>
#include <iostream>

class Inverter
{
       public:
        int invID;   // 인버터 번호, 0번부터 시작
        int dailyYeild;
        int totalYeild;
        int dcCurrentA;
        int dcCurrentB;
        int dcVoltageA;
        int dcVoltageB;
        int dcPowerA;
        int dcPowerB;
        int dcCurrent;
        int dcVoltage;
        int dcPower;
        int acPower;
        int acCurrent;
        int acVoltage1;
        int acVoltage2;
        int acVoltage3;
        int acFrequency;
        int operatingStatus;

       Inverter(int no)
       {
           invID = no;
           dailyYeild = 0;
           totalYeild = 0;
           dcCurrentA = 0;
           dcCurrentB = 0;
           dcVoltageA = 0;
           dcVoltageB = 0;
           dcPowerA = 0;
           dcPowerB = 0;
           dcPowerA = 0;
           dcCurrent = 0;
           dcVoltage = 0;
           dcPower = 0;
           acPower = 0;
           acCurrent = 0;
           acVoltage1 = 0;
           acVoltage2 = 0;
           acVoltage3 = 0;
           acFrequency = 0;
           operatingStatus = 0;
       }

       // Remaining implementation of Person class.
};

union uni{
    char SEND;
    unsigned char uSEND;
};

char SEND1[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x77, 0x43, 0x00, 0x0a };   // Yield Daily, Total (30531)
char SEND2[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x78, 0x31, 0x00, 0x46 };   // DC Input A,V,W A (30769)
char SEND3[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x78, 0xed, 0x00, 0x0a };   // DC Input A,V,W B (30957)
char SEND4[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x9c, 0x5d, 0x00, 0x0a };   // Operating status (40029)

char TSEND[4][12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x77, 0x43, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x78, 0x31, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x78, 0xed, 0x00, 0x0a,0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x9c, 0x5d, 0x00, 0x0a};




        // 인버터 저장할 변수 클래스 생성
Inverter* inv[20]; //= new Inverter[20];

int selectSendMsgType = 1;

QString ADDR_RTU = "http://solarapi.hstec.kr:80/rtu/start";
QString ADDR_INV = "http://solarapi.hstec.kr:80/invert";
        //public String ADDR_HSTEC = "http://hstec.kr:7780/";
        //public String ADDR_INV = "http://192.168.50.51/sunapi/public/invert"; // mint test server

QString invIpTest = "hstec8646.iptime.org";
QString invIP[20]  = {
            /*"192.168.0.2",
            "192.168.0.3",
            "192.168.0.4",
            "192.168.0.5",
            "192.168.0.6",
            "192.168.0.7",
            "192.168.0.8",
            "192.168.0.9",
            "192.168.0.10",
            "192.168.0.11",
            "192.168.0.12",
            "192.168.0.13",
            "192.168.0.14",
            "192.168.0.15",
            "192.168.0.16",
            "192.168.0.17",
            "192.168.0.18",
            "192.168.0.19",
            "192.168.0.20",
            "192.168.0.21"*/

            //"172.30.1.5"

            "192.168.0.101",
            "192.168.0.102",
            "192.168.0.103",
            "192.168.0.104",
            "192.168.0.105",
            "192.168.0.106",
            "192.168.0.107",
            "192.168.0.108",
            "192.168.0.109",
            "192.168.0.110",
            "192.168.0.111",
            "192.168.0.112",
            "192.168.0.113",
            "192.168.0.114",
            "192.168.0.115",
            "192.168.0.116",
            "192.168.0.117",
            "192.168.0.118",
            "192.168.0.119",
            "192.168.0.120"
        };

//int port = 502;

char sdata[70] = {0,};// = new char[70];    // 구서버 송신 버퍼
char checksum = 0;              // 구서버 송신 체크섬

int plantNumber=7777;
int invCount=1;

int sendTimeInverter = 0;
int sendTimeServer = 0;

int NCSQ=0;
int jj=0;

bool first=true;
bool toggle=true;

bool wcdma_error=false;
int wcdma_count=0;

bool wfirst=true;
QString wread;

#endif // DATAH_H

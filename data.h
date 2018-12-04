#ifndef DATAH_H
#define DATAH_H

#include <QString>
#include <stdio.h>
#include <iostream>

/*


    struct Inverter
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

Inverter* inv[20]; //= new Inverter[20];

char TSEND[4][12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x77, 0x43, 0x00, 0x0a,  // Yield Daily, Total (30531)
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x78, 0x31, 0x00, 0x46,  // DC Input A,V,W A (30769)
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x78, 0xed, 0x00, 0x0a,  // DC Input A,V,W B (30957)
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x9c, 0x5d, 0x00, 0x0a}; // Operating status (40029)

char sdata[70] = {0,};// = new char[70];    // 구서버 송신 버퍼
char checksum = 0;              // 구서버 송신 체크섬

int plantNumber=7777;
int invCount=1;

bool first=true;
bool toggle=true;

bool wcdma_error=false;
int wcdma_count=0;

bool wfirst=true;
QString wread;

bool reboot=false;
int NCSQ=0;

*/

struct Inverter
{
       public:
        int invID;   // 인버터 번호, 0번부터 시작
        int dailyYeild;
        int totalYeild;

        int dcCurrentA;
        int dcCurrentB;
        int dcCurrentC;
        int dcCurrentD;
        int dcCurrentE;
        int dcCurrentF;

        int dcVoltageA;
        int dcVoltageB;

        int dcPowerA;
        int dcPowerB;
        int dcPowerC;
        int dcPowerD;
        int dcPowerE;
        int dcPowerF;

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
           dcCurrentC = 0;
           dcCurrentD = 0;
           dcCurrentE = 0;
           dcCurrentF = 0;
           dcVoltageA = 0;
           dcVoltageB = 0;
           dcPowerA = 0;
           dcPowerB = 0;
           dcPowerC = 0;
           dcPowerD = 0;
           dcPowerE = 0;
           dcPowerF = 0;
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

#endif // DATAH_H

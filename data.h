#ifndef DATAH_H
#define DATAH_H

#include <QString>
#include <stdio.h>
#include <iostream>

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
        int operatingStatus1;

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
           operatingStatus1 = 0;
       }
};



#endif // DATAH_H

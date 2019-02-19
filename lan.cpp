#include "mainwindow.h"
#include "tcpclient.h"
#include "data.h"
#include "ui_mainwindow.h"

extern int plantNumber;
extern int invCount;

extern char sdata[70];// = new char[70];    // 구서버 송신 버퍼
extern char checksum;              // 구서버 송신 체크섬

extern Inverter* inv[20];
extern char black;

//checksum
void addPacket(int cnt, int value)
{
    sdata[cnt] = (char)(value&0xff);
    checksum ^= sdata[cnt];
}


// INTERNET을 통한 전송
void MainWindow::SendServerHstec()
{
    int eeport = 0;

    if(plantNumber<7500){
        eeport = 7777;

    } else if(plantNumber<8000){
        eeport = 7778;

    } else if(plantNumber<8500){
        eeport = 7779;

    } else if(plantNumber<9000){
        eeport = 7780;

    } else if(plantNumber<9500){
        eeport = 7781;

    }



    for (int i = 0; i < invCount; i++)
    {
        if(inv[i]->operatingStatus == 0x571 || inv[i]->operatingStatus == 0xFFFD)
            inv[i]->acPower=0;

        addPacket(0, 0x55);
        checksum = 0;
        addPacket(1, 0x54);
        addPacket(2, (plantNumber / 0x100));
        addPacket(3, (plantNumber % 0x100));
        addPacket(4, (inv[i]->invID + 0x31));
        addPacket(5, 0);        // adc0
        addPacket(6, 25);       // adc1
        addPacket(7, 0);        // adc0
        addPacket(8, 0);        // adc0
        addPacket(9, black);        // blackout im_key
        addPacket(10, inv[i]->operatingStatus / 0x100);    // inverter_status
        addPacket(11, inv[i]->operatingStatus % 0x100);    // inverter_status
        addPacket(12, inv[i]->operatingStatus1 / 0x100);    // inverter_status
        addPacket(13, inv[i]->operatingStatus1 / 0x100);    // inverter_status
        addPacket(14, 0);    // inverter_status
        addPacket(15, 0);    // inverter_status
        addPacket(16, 0);    // inverter_status
        addPacket(17, 0);    // inverter_status
        addPacket(18, (inv[i]->dcVoltage / 100) / 0x100);
        addPacket(19, (inv[i]->dcVoltage / 100) % 0x100);
        addPacket(20, (inv[i]->dcCurrent / 1000) / 0x100);
        addPacket(21, (inv[i]->dcCurrent / 1000) % 0x100);

        addPacket(22, (inv[i]->acVoltage1 / 100) / 0x100);
        addPacket(23, (inv[i]->acVoltage1 / 100) % 0x100);
        addPacket(24, (inv[i]->acVoltage2 / 100) / 0x100);
        addPacket(25, (inv[i]->acVoltage2 / 100) % 0x100);
        addPacket(26, (inv[i]->acVoltage3 / 100) / 0x100);
        addPacket(27, (inv[i]->acVoltage3 / 100) % 0x100);

        addPacket(28, (inv[i]->acCurrent / 1000) / 0x100);
        addPacket(29, (inv[i]->acCurrent / 1000) % 0x100);
        addPacket(30, (inv[i]->acCurrent / 1000) / 0x100);
        addPacket(31, (inv[i]->acCurrent / 1000) % 0x100);
        addPacket(32, (inv[i]->acCurrent / 1000) / 0x100);
        addPacket(33, (inv[i]->acCurrent / 1000) % 0x100);

        addPacket(34, (inv[i]->acFrequency / 10) / 0x100);
        addPacket(35, (inv[i]->acFrequency / 10) % 0x100);

        addPacket(36, (inv[i]->acVoltage1 / 100) / 0x100);
        addPacket(37, (inv[i]->acVoltage1 / 100) % 0x100);
        addPacket(38, (inv[i]->acVoltage2 / 100) / 0x100);
        addPacket(39, (inv[i]->acVoltage2 / 100) % 0x100);
        addPacket(40, (inv[i]->acVoltage3 / 100) / 0x100);
        addPacket(41, (inv[i]->acVoltage3 / 100) % 0x100);

        addPacket(42, (inv[i]->acCurrent / 1000) / 0x100);
        addPacket(43, (inv[i]->acCurrent / 1000) % 0x100);
        addPacket(44, (inv[i]->acCurrent / 1000) / 0x100);
        addPacket(45, (inv[i]->acCurrent / 1000) % 0x100);
        addPacket(46, (inv[i]->acCurrent / 1000) / 0x100);
        addPacket(47, (inv[i]->acCurrent / 1000) % 0x100);

        addPacket(48, (inv[i]->acFrequency / 10) / 0x100);
        addPacket(49, (inv[i]->acFrequency / 10) % 0x100);

        addPacket(50, ((inv[i]->dcPower) / 100) / 0x100);
        addPacket(51, ((inv[i]->dcPower) / 100) % 0x100);

        addPacket(52, (inv[i]->totalYeild >> 24) & 0xff);
        addPacket(53, (inv[i]->totalYeild >> 16) & 0xff);
        addPacket(54, (inv[i]->totalYeild >> 8) & 0xff);
        addPacket(55, inv[i]->totalYeild & 0xff);

        addPacket(56, (inv[i]->acPower / 100) / 0x100);
        addPacket(57, (inv[i]->acPower / 100) % 0x100);
        addPacket(58, (inv[i]->acPower / 100) / 0x100);
        addPacket(59, (inv[i]->acPower / 100) % 0x100);

        addPacket(60, inv[i]->dailyYeild / 0x100);
        addPacket(61, inv[i]->dailyYeild % 0x100);

        addPacket(62, 0);   // pf
        addPacket(63, 0);   // pf

        sdata[64] = checksum;
        sdata[65] = 0xaa;


        qDebug()<<"s : "<<hex<<sdata;
        SendMessageHstec("hstec.kr", eeport, sdata,sizeof(sdata));

     }
}

//LAN을 통한 전송 부분
void MainWindow::SendMessageHstec(QString server, int port, char* data,int size)
{
    TcpClient *client = new TcpClient();

    if(client->TcpConnect(server,port)==true)
    {
        client->onConnectServer();
        client->sendRequst(data,size);

        ui->textBrowser->clear();
        ui->textBrowser->append("HSTEC send ok!!");
    }
    else
    {
        ui->textBrowser->clear();
        ui->textBrowser->append("HSTEC send failed..");
    }

    client->TcpDisconnect();
}


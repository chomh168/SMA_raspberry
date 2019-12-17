
#include "mainwindow.h"
#include "data.h"
#include "ui_mainwindow.h"

#define INF 1

extern int plantNumber;
extern int invCount;


bool lte_error; //wcdma 에러가 발생했는지(5번 발생시 flag down->count_error flag up)
int lte_count;

extern bool reboot;
extern int NCSQ;

extern Inverter* inv[20];


extern bool black;
bool lcount_error; //5번 연속시도 후 실패시 flag up
int lcheck_count;


//WCDMA를 통한 전송
void MainWindow::SendLTE()
{

    char ATE[7] = {0x41, 0x54, 0x45, 0x30, 0x0D, 0x0A};
    char CSQ[8] = {0x41, 0x54, 0x2B, 0x43, 0x53, 0x51, 0x0D};


    char STAT[16]={0x41, 0x54, 0x2A, 0x41, 0x55, 0x54, 0x4F, 0x43, 0x4F, 0x4E, 0x4E, 0x3D, 0x31, 0x0D, 0x0A};
    char RNDISDATA[16]={0x41, 0x54, 0x2A, 0x52, 0x4E, 0x44, 0x49, 0x53, 0x44, 0x41, 0x54, 0x41, 0x3D, 0x31, 0x0D, 0x0A};
    //13:mode 1-on,0-off
    char WSOCR[30]={0x41, 0x54, 0x2B, 0x57, 0x53, 0x4F, 0x43, 0x52, 0x3D, 0x30, 0x2C, 0x68, 0x73, 0x74, 0x65, 0x63, 0x2E, 0x6B, 0x72, 0x2C, 0x37, 0x37, 0x37, 0x37, 0x2C, 0x31, 0x2C, 0x31, 0x0D, 0x0A};
    //20~23:port

    char WSOCO[12]={0x41, 0x54, 0x2B, 0x57, 0x53, 0x4F, 0x43, 0x4F, 0x3D, 0x30, 0x0D};
    char WSOWR[256]={0x41, 0x54, 0x2B, 0x57, 0x53, 0x4F, 0x57, 0x52, 0x3D, 0x30, 0x2C, 0x31, 0x33, 0x36, 0x2C};

    char WSOCL[12]={0x41, 0x54, 0x2B, 0x57, 0x53, 0x4F, 0x43, 0x4C, 0x3D, 0x30, 0x0D};


    if(plantNumber<7500){
        //7777;
        WSOCR[22]=0x37;
        WSOCR[23]=0x37;

    } else if(plantNumber<8000){
        //7778;

        WSOCR[22]=0x37;
        WSOCR[23]=0x38;


    } else if(plantNumber<8500){
        //7779;

        WSOCR[22]=0x37;
        WSOCR[23]=0x39;


    } else if(plantNumber<9000){
        //7780;

        WSOCR[22]=0x38;
        WSOCR[23]=0x30;


    } else if(plantNumber<10000){
        //7781;

        WSOCR[22]=0x38;
        WSOCR[23]=0x31;

    }

    qDebug()<<"tcpopen : "<<WSOCR;


    if(wiringPiSetup() == -1)
    {
        qDebug()<<"err:not setup";
    }
    else
    {
        for(int i =0; i<=8; i++)
        {
            system("sudo chmod 777 /dev/ttyAMA0");

            if(i==0)
            {
                luart_ch(ATE,i);
                qDebug()<<"ATE";
            }
            else if(i==1)
            {
                luart_ch(CSQ,i);
                qDebug()<<"CSQ";
            }
            else if(i==2)
            {
                luart_ch(STAT,i);
                qDebug()<<"AUTOCONN";
            }
            else if(i==3)
            {
                RNDISDATA[13]=0x31;
                luart_ch(RNDISDATA,i);
                qDebug()<<"RNDISDATA OPEN";
            }
            else if(i==4)
            {
                luart_ch(WSOCR,i);
                qDebug()<<"WSOCR";
            }
            else if(i==5)
            {//WSOCO
                luart_ch(WSOCO,i);
                qDebug()<<"WSOCO";
            }
            else if(i==6)
            {
                lsend_append(WSOWR);
                qDebug()<<"WSOWR : "<<WSOWR;
            }

            else if(i==7)
            {
                luart_ch(WSOCL,i);
                qDebug()<<"WSOCL";
            }
            else if(i==8)
            {
                RNDISDATA[13]=0x30;
                luart_ch(RNDISDATA,i);
                qDebug()<<"RNDISDATA OFF";

            }
        }
    }
}

//유아트 전송
QString MainWindow::luart_ch(char *ch, int state)
{
    char rxbuffer[1024] = {0,};
    int fd;
    int count=0;

    char buf;

    if((fd=serialOpen("/dev/serial0",115200))<0)
    {
        qDebug()<<"err:not Open";
    }

    system("sudo chmod 777 /dev/ttyAMA0");
    serialPuts(fd,ch);


    if(state == 1)
    {
        QThread::sleep(1);
        while(serialDataAvail(fd)!='\0')
        {
            system("sudo chmod 777 /dev/ttyAMA0");
            buf = serialGetchar(fd);
            rxbuffer[count]=buf;
            count++;
        }

        serialFlush(fd);
        serialClose(fd);

        qDebug()<<"Msg :"<<rxbuffer;

        QString lcsq = QString(rxbuffer);

        if(lcsq.length()==7)
            NCSQ=0;
        else
            NCSQ = lcsq[8].digitValue()*10 + lcsq[9].digitValue();


        return lcsq;
    }

    else if(state == 3 || state == 4 || state == 5)
    {
        QThread::sleep(1);
        while(serialDataAvail(fd)!='\0')
            {
                system("sudo chmod 777 /dev/ttyAMA0");
                buf = serialGetchar(fd);
                rxbuffer[count]=buf;
                count++;
            }

            QString feed = QString(rxbuffer);

            qDebug()<<"open : "<<rxbuffer;

            if(feed.indexOf("ERROR")!=-1) //에러 미발생
            {
                lte_error=true;
                lcount_error=false;
            }

        serialFlush(fd);
        serialClose(fd);

        return 0;
    }



    else if(state == 6)
    {
        QThread::sleep(1);

        while(serialDataAvail(fd)!='\0')
        {
            system("sudo chmod 777 /dev/ttyAMA0");
            buf = serialGetchar(fd);
            rxbuffer[count]=buf;
            count++;
        }

        QString feed = QString(rxbuffer);


        qDebug()<<"content : "<<feed;


        serialFlush(fd);
        serialClose(fd);

        return feed;
    }

    else if (state==7||state==8)
    {
        QThread::sleep(1);

        serialFlush(fd);
        serialClose(fd);

        return 0;
    }

    else
    {
        QThread::sleep(1);

        serialFlush(fd);
        serialClose(fd);

        return 0;
    }

}

//서버 전송
void MainWindow::lsend_append(char *WSOWR)
{
    char buffer[140]={0,};
    char ccs=0;
    //black=0;

    bool error_flag = false;

    for(int i = 0;i<invCount;i++)
    {

           //black=0; //test

        if(inv[i]->operatingStatus == 0x571 || inv[i]->operatingStatus == 0xFFFD) //acpower 값이상 예외처리  예외처리
            inv[i]->acPower=0;

        ccs = 0;
        ccs ^= 'T';
        ccs ^= (char)(plantNumber/0x100);
        ccs ^= (char)(plantNumber%0x100);
        ccs ^= (inv[i]->invID+0x31);

        //inv_number--;
        ccs ^= (char)19;
        ccs ^= (char)0;
        ccs ^= (char)19;
        ccs ^= (char)0;
        ccs ^= (char)black;
        ccs ^= (char)(inv[i]->operatingStatus/0x100);
        ccs ^= (char)(inv[i]->operatingStatus%0x100);
        ccs ^= (char)(inv[i]->operatingStatus1/0x100);
        ccs ^= (char)(inv[i]->operatingStatus1%0x100);
        ccs ^= (char)(inv[i]->operatingStatus2/0x100);
        ccs ^= (char)(inv[i]->operatingStatus2%0x100);
        ccs ^= (char)(inv[i]->operatingStatus3/0x100);
        ccs ^= (char)(inv[i]->operatingStatus3%0x100);

        ccs ^= (char)((inv[i]->dcVoltage / 100)/0x100);
        ccs ^= (char)((inv[i]->dcVoltage / 100)%0x100);
        ccs ^= (char)((inv[i]->dcCurrent / 1000)/0x100);
        ccs ^= (char)((inv[i]->dcCurrent / 1000)%0x100);

        ccs ^= (char)((inv[i]->acVoltage1 / 100)/0x100);
        ccs ^= (char)((inv[i]->acVoltage1 / 100)%0x100);
        ccs ^= (char)((inv[i]->acVoltage2 / 100)/0x100);
        ccs ^= (char)((inv[i]->acVoltage2 / 100)%0x100);
        ccs ^= (char)((inv[i]->acVoltage3 / 100) /0x100);
        ccs ^= (char)((inv[i]->acVoltage3 / 100) %0x100);
        ccs ^= (char)((inv[i]->acCurrent / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent / 1000)%0x100);
        ccs ^= (char)((inv[i]->acCurrent2 / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent2 / 1000)%0x100);

        ccs ^= (char)((inv[i]->acCurrent3 / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent3 / 1000)%0x100);
        ccs ^= (char)((inv[i]->acFrequency/10)/0x100);
        ccs ^= (char)((inv[i]->acFrequency/10)%0x100);

        ccs ^= (char)((inv[i]->acVoltage1 / 100)/0x100);
        ccs ^= (char)((inv[i]->acVoltage1 / 100)%0x100);
        ccs ^= (char)((inv[i]->acVoltage2 / 100)/0x100);
        ccs ^= (char)((inv[i]->acVoltage2 / 100)%0x100);
        ccs ^= (char)((inv[i]->acVoltage3 / 100) /0x100);
        ccs ^= (char)((inv[i]->acVoltage3 / 100) %0x100);
        ccs ^= (char)((inv[i]->acCurrent / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent / 1000)%0x100);
        ccs ^= (char)((inv[i]->acCurrent2 / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent2 / 1000)%0x100);

        ccs ^= (char)((inv[i]->acCurrent3 / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent3 / 1000)%0x100);
        ccs ^= (char)((inv[i]->acFrequency/10)/0x100);
        ccs ^= (char)((inv[i]->acFrequency/10)%0x100);

        ccs ^= (char)((inv[i]->dcPower / 100)/0x100);
        ccs ^= (char)((inv[i]->dcPower / 100)%0x100);
        ccs ^= (char)((inv[i]->totalYeild>>24)&0xff);
        ccs ^= (char)((inv[i]->totalYeild>>16)&0xff);
        ccs ^= (char)((inv[i]->totalYeild>>8)&0xff);
        ccs ^= (char)(inv[i]->totalYeild&0xff);
        ccs ^= (char)((inv[i]->acPower / 100)/0x100);
        ccs ^= (char)((inv[i]->acPower / 100)%0x100);
        ccs ^= (char)((inv[i]->acPower / 100)/0x100);
        ccs ^= (char)((inv[i]->acPower / 100)%0x100);

        ccs ^= (char)(inv[i]->dailyYeild/0x100);
        ccs ^= (char)(inv[i]->dailyYeild%0x100);
        ccs ^= (char)(0);
        ccs ^= (char)(0);

    sprintf(buffer,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
            0xFF,
            0xFF,
            0x55,
            'T',
            (unsigned char)(plantNumber/0x100),
            (unsigned char)(plantNumber%0x100),
            (inv[i]->invID+0x31),
            (unsigned char)19,  //0 1
            (unsigned char)0, //2 3
            (unsigned char)19,  //4 5
            (unsigned char)0,  //6 7
            (unsigned char)black,  //8 9
            (unsigned char)(inv[i]->operatingStatus/0x100), //10 11
            (unsigned char)(inv[i]->operatingStatus%0x100), //12 13
            (unsigned char)(inv[i]->operatingStatus1/0x100), //14
            (unsigned char)(inv[i]->operatingStatus1%0x100), //16
            (unsigned char)(inv[i]->operatingStatus2/0x100), //18
            (unsigned char)(inv[i]->operatingStatus2%0x100),  //20
            (unsigned char)(inv[i]->operatingStatus3/0x100),  //22
            (unsigned char)(inv[i]->operatingStatus3%0x100),  //24
            (unsigned char)((inv[i]->dcVoltage / 100)/0x100),  //26
            (unsigned char)((inv[i]->dcVoltage / 100)%0x100),  //28
            (unsigned char)((inv[i]->dcCurrent / 1000)/0x100),  //30
            (unsigned char)((inv[i]->dcCurrent / 1000)%0x100),  //32
            (unsigned char)((inv[i]->acVoltage1 / 100)/0x100),  //34
            (unsigned char)((inv[i]->acVoltage1 / 100)%0x100),  //36
            (unsigned char)((inv[i]->acVoltage2 / 100)/0x100),  //38
            (unsigned char)((inv[i]->acVoltage2 / 100)%0x100),  //40
            (unsigned char)((inv[i]->acVoltage3 / 100)/0x100),  //42
            (unsigned char)((inv[i]->acVoltage3 / 100)%0x100),  //44
            (unsigned char)((inv[i]->acCurrent / 1000)/0x100),  //46
            (unsigned char)((inv[i]->acCurrent / 1000)%0x100),  //48
            (unsigned char)((inv[i]->acCurrent2 / 1000)/0x100),  //50
            (unsigned char)((inv[i]->acCurrent2 / 1000)%0x100),  //52
            (unsigned char)((inv[i]->acCurrent3 / 1000)/0x100),  //54
            (unsigned char)((inv[i]->acCurrent3 / 1000)%0x100),  //56
            (unsigned char)((inv[i]->acFrequency/10)/0x100),  //58
            (unsigned char)((inv[i]->acFrequency/10)%0x100),  //60
            (unsigned char)((inv[i]->acVoltage1 / 100)/0x100),  //62
            (unsigned char)((inv[i]->acVoltage1 / 100)%0x100),  //64
            (unsigned char)((inv[i]->acVoltage2 / 100)/0x100),  //66
            (unsigned char)((inv[i]->acVoltage2 / 100)%0x100),  //68
            (unsigned char)((inv[i]->acVoltage3 / 100)/0x100),  //70
            (unsigned char)((inv[i]->acVoltage3 / 100)%0x100),  //72
            (unsigned char)((inv[i]->acCurrent / 1000)/0x100),  //74
            (unsigned char)((inv[i]->acCurrent / 1000)%0x100),  //76
            (unsigned char)((inv[i]->acCurrent2 / 1000)/0x100),  //78
            (unsigned char)((inv[i]->acCurrent2 / 1000)%0x100),  //80
            (unsigned char)((inv[i]->acCurrent3 / 1000)/0x100),  //82
            (unsigned char)((inv[i]->acCurrent3 / 1000)%0x100),  //84
            (unsigned char)((inv[i]->acFrequency/10)/0x100),  //86
            (unsigned char)((inv[i]->acFrequency/10)%0x100),  //88
            (unsigned char)((inv[i]->dcPower / 100)/0x100),  //90
            (unsigned char)((inv[i]->dcPower / 100)%0x100),  //92
            (unsigned char)((inv[i]->totalYeild>>24)&0xff),  //94
            (unsigned char)((inv[i]->totalYeild>>16)&0xff),  //96
            (unsigned char)((inv[i]->totalYeild>>8)&0xff),  //98
            (unsigned char)(inv[i]->totalYeild&0xff),  //100
            (unsigned char)((inv[i]->acPower / 100)/0x100),  //102
            (unsigned char)((inv[i]->acPower / 100)%0x100),  //104
            (unsigned char)((inv[i]->acPower / 100)/0x100),  //106
            (unsigned char)((inv[i]->acPower / 100)%0x100),  //108
            (unsigned char)(inv[i]->dailyYeild/0x100),  //110
            (unsigned char)(inv[i]->dailyYeild%0x100),  //112
            (unsigned char)(0),  //114
            (unsigned char)(0),  //116
            ccs,  //118
            0xaa  //120
            );

        for(int j = 15 ; j<136+15;j++)
        {
            WSOWR[j] = buffer[j-15];
        }

        WSOWR[151] = 0x0d;
        WSOWR[152] = 0x0a;

        QString feed = uart_ch(WSOWR,6);

        if(feed.indexOf("OK") == -1)
        {
            if(feed.indexOf("5552ffff")>0)
            {
                qDebug()<<"FFFF";
                reboot=true;
            }

            error_flag |= true;
            break; 
        }

    }

    if(error_flag == true)
    {
        lte_error=true;

        if(lte_count==5)
        {
            lte_count=0;
            lte_error=false;
            lcount_error=true;
        }

        lte_count++;
    }
    else
    {
        lte_error=false;
        lcount_error=false;
        lte_count=0;
    }

}



void MainWindow::lsend_ok()
{
    ui->label_5->setText("CSQ : "+QString::number(NCSQ));

    if(lte_error==true)
    {
        if(sms_watcher.isRunning()==true) QThread::sleep(20);//보통의 루틴 시간

        //setFileLog("lte error");
        QFuture<void> th5 = QtConcurrent::run(MainWindow::SendLTE);
        lsend_watcher.setFuture(th5);
    }
    else
    {
        ui->textBrowser->clear();
        if(lcount_error==false){
            ui->textBrowser->append("LTE send ok!!");
            digitalWrite(0,0);

            lcheck_count=0;
        }
        else {
            ui->textBrowser->append("LTE send error");
            lcheck_count++;

            if(lcheck_count==4)
            {
                digitalWrite(0,(1)); //외부 버튼을 이용한 리셋 기능
                QThread::sleep(1);
                digitalWrite(0,(0));
                lcheck_count=0;
            }

        }
    }

    if(reboot==true)
    {
        //setFileLog("reboot");
        system("reboot");
    }
}

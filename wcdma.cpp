#include "mainwindow.h"
#include "data.h"

extern int plantNumber;
extern int invCount;


extern bool wcdma_error;
extern int wcdma_count;

extern bool wfirst;
extern QString wread;

extern bool reboot;
extern int NCSQ;

extern Inverter* inv[20];

//문자 전송 부분
void MainWindow::SMSReceive()
{
    char CMGR[12] = {0x41, 0x54 , 0x2B , 0x43 , 0x4D , 0x47 , 0x52 , 0x3D , 0x30 , 0x0D};
    char CNUM[10] = {0x41, 0x54 , 0x24 , 0x24 , 0x43 , 0x4e , 0x55 , 0x4d , 0x0d};
    char CMGD[12] = {0x41, 0x54, 0x2B, 0x43, 0x4D, 0x47, 0x44, 0x3D, 0x30, 0x0D};
    char SMSW[60] = {0x41, 0x54 , 0x24 , 0x24, 0x53 , 0x4D , 0x53 , 0x57 , 0x3D , 0x22 , 0x30 , 0x31 , 0x30 , 0x35 , 0x35 , 0x37 , 0x37 , 0x37 , 0x36 , 0x32 , 0x37 , 0x22 , 0x2C , 0x22 , 0x30 , 0x31 , 0x32 , 0x32 , 0x39 , 0x31 , 0x38 , 0x36 , 0x36 , 0x32 , 0x32 , 0x22 , 0x2C , 0x30 , 0x2C , 0x30 , 0x2C , 0x30 , 0x2C , 0x31 , 0x33 , 0x32 , 0x0D};
    char MSG[6] = {0x4F, 0x4B, 0x1A, 0x0D};
    //char CMGF[10] = {0x41, 0x54 , 0x2b , 0x43 , 0x4d , 0x47 , 0x46 , 0x3d , 0x31 , 0x0d};
    char CSMP[16] = {0x41, 0x54, 0x2B, 0x43, 0x53, 0x4D, 0x50, 0x3D, 0x31, 0x2C, 0x2C, 0x30, 0x2C, 0x30, 0x0d};

    char rxbuffer[1024] = {0,};
    int fd;
    int count=0;
    char buf;

    bool reboot=false;

    if((fd=serialOpen("/dev/serial0",115200))<0)
    {
       qDebug()<<"err:not Open";
    }

        serialPuts(fd,CSMP); // CNUM

        QThread::sleep(2);
        serialFlush(fd);


        serialPuts(fd,CNUM); // CNUM

        QThread::sleep(2);

        while(serialDataAvail(fd)!=NULL)
        {
            system("sudo chmod 777 /dev/ttyAMA0");
            buf = serialGetchar(fd);
            rxbuffer[count]=buf;
            count++;
        }

        serialFlush(fd);
        QString str = QString(rxbuffer);

        //initialization
        memset((void*)&rxbuffer, 0, sizeof(rxbuffer));
        count = 0;

        qDebug()<<"after CNUM : "<<str;

        if(str.indexOf("8212")>0)
        {
            for(int i = 0;i<8;i++)
            {
                SMSW[27+i] = str.at(str.indexOf("8212")+4+i).toLatin1();
            }
        }


        serialPuts(fd,CMGR); // CMGR

        qDebug()<<SMSW;

        QThread::sleep(2);

        while(serialDataAvail(fd)!=NULL)
        {
            system("sudo chmod 777 /dev/ttyAMA0");
            buf = serialGetchar(fd);
            rxbuffer[count]=buf;
            count++;
        }

        serialFlush(fd);

        QString feed = QString(rxbuffer);
            qDebug()<<"read : "<<feed;

        if(feed.indexOf("010")>0)
        {
            for(int i =0;i<11;i++)
            {
                SMSW[10+i] = feed.at(feed.indexOf("010")+i).toLatin1();
            }

            if(feed.indexOf("5339383230")>0)  //received MESSAGE
            {
                serialPuts(fd,SMSW);

                QThread::sleep(2);
                while(serialDataAvail(fd)!=NULL)
                {
                    system("sudo chmod 777 /dev/ttyAMA0");
                    buf = serialGetchar(fd);
                    rxbuffer[count]=buf;
                    count++;
                }
                serialFlush(fd);
                qDebug()<<rxbuffer;

                serialPuts(fd,MSG);

                //reboot=true;
            }

            QThread::sleep(1);

            serialPuts(fd,CMGD);
        }

    QThread::sleep(1);

    serialFlush(fd);
    serialClose(fd);

    if(reboot==true)
        system("reboot");
}

//WCDMA를 통한 전송
void MainWindow::SendWCDMA()
{
    //while(sms_watcher.isRunning()==true) QThread::sleep(20);

    int eeport = 0;

    char ATE[7] = {0x41, 0x54, 0x45, 0x30, 0x0D, 0x0A};
    char CSQ[8] = {0x41, 0x54, 0x2B, 0x43, 0x53, 0x51, 0x0D};
    char TCPTYPE[16] = {0x41, 0x54 , 0x24 , 0x24 , 0x54 , 0x43 , 0x50 , 0x54 , 0x59 , 0x50 , 0x45 , 0x3D , 0x32 , 0x0D , 0x0A};
    char PPPOPEN[13] = {0x41 , 0x54 , 0x24 , 0x24 , 0x50 , 0x50 , 0x50 , 0x4F , 0x50 , 0x45 , 0x4E , 0x0D, 0x0A};
    //char TCPOPEN[50] = {0x41 , 0x54 , 0x24 , 0x24 , 0x54 , 0x43 , 0x50 , 0x4F , 0x50 , 0x45 , 0x4E , 0x3D  , 0x32 , 0x32 , 0x30 , 0x2E , 0x31 , 0x32 , 0x32 , 0x2E , 0x32 , 0x30 , 0x33 , 0x2E , 0x31 , 0x31 , 0x31 , 0x2C , 0x37 , 0x37 , 0x37 , 0x38 , 0x0D , 0x0A};
    char TCPOPEN[50] = {0x41 , 0x54 , 0x24 , 0x24 , 0x54 , 0x43 , 0x50 , 0x4F , 0x50 , 0x45 , 0x4E , 0x3D  , 0x31 , 0x32 , 0x31 , 0x2E , 0x31 , 0x35 , 0x39 , 0x2E , 0x33 , 0x30 , 0x2E , 0x31, 0x35 , 0x2C , 0x37 , 0x37}; //, 0x37 , 0x37 , 0x37 , 0x38 , 0x0D , 0x0A};

    char TCPWRITE[256] = {0x41 , 0x54 , 0x24 , 0x24 , 0x54 , 0x43 , 0x50 , 0x57 , 0x52 , 0x49 , 0x54 , 0x45 , 0x3D};//, 0xFF, 0xFF};

    char TCPREAD[15] = {0x41, 0x54, 0x24, 0x24, 0x54, 0x43, 0x50, 0x52, 0x45, 0x41, 0x44, 0x0d, 0x0a};

    char TCPCLOSE[15] = {0x41 , 0x54 , 0x24 , 0x24 , 0x54 , 0x43 , 0x50 , 0x43 , 0x4C , 0x4F , 0x53 , 0x45 , 0x0D};
    char PPPCLOSE[15] = {0x41 , 0x54 , 0x24 , 0x24 , 0x50 , 0x50 , 0x50 , 0x43 , 0x4C , 0x4F , 0x53 , 0x45 , 0x0D};



    if(plantNumber<7500){
        eeport = 7777;
        TCPOPEN[23]=0x37;
        TCPOPEN[24]=0x37;
        TCPOPEN[25]=0x0D;
        TCPOPEN[25]=0x0A;

    } else if(plantNumber<8000){
        eeport = 7778;
        TCPOPEN[28]=0x37;
        TCPOPEN[29]=0x38;
        TCPOPEN[30]=0x0D;
        TCPOPEN[31]=0x0A;

    } else if(plantNumber<8500){
        eeport = 7779;
        TCPOPEN[28]=0x37;
        TCPOPEN[29]=0x39;
        TCPOPEN[30]=0x0D;
        TCPOPEN[31]=0x0A;

    } else if(plantNumber<9000){
        eeport = 7780;
        TCPOPEN[28]=0x38;
        TCPOPEN[29]=0x30;
        TCPOPEN[30]=0x0D;
        TCPOPEN[31]=0x0A;

    }

    qDebug()<<"tcpopen : "<<TCPOPEN;


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
                uart_ch(ATE,i);
                qDebug()<<"ATE";
            }
            else if(i==1)
            {
                uart_ch(CSQ,i);
                qDebug()<<"CSQ";
            }
            else if(i==2)
            {
                uart_ch(TCPTYPE,i);
                qDebug()<<"TYPE";
            }
            else if(i==3)
            {
                uart_ch(PPPOPEN,i);
                qDebug()<<"OPEN";
            }
            else if(i==4)
            {
                uart_ch(TCPOPEN,i);
                qDebug()<<"OPEN2";


            }
            else if(i==5)
            {
                send_append(TCPWRITE);
                qDebug()<<"tcpwrite : "<<TCPWRITE;
            }

            //read??
            else if(i==6)
            {
                QString rec = uart_ch(TCPREAD,i);
                qDebug()<<"READ";

                if(rec.indexOf("OK")!=-1)
                {
                    if(wfirst==true)
                    {
                        wfirst=false;
                        wread = rec;
                    }
                    else
                    {
                        if(wread!=rec)
                        {
                            reboot=true;
                        }

                    }
                }

            }

            else if(i==7)
            {
                uart_ch(TCPCLOSE,i);
                qDebug()<<"CLOSE";
            }
            else if(i==8)
            {
                uart_ch(PPPCLOSE,i);
                qDebug()<<"CLOSE2";

            }
        }
    }
}

//유아트 전송
QString MainWindow::uart_ch(char *ch, int state)
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
        while(serialDataAvail(fd)!=NULL)
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

    else if(state == 3 || state == 4)
    {
        QThread::sleep(5);
            while(serialDataAvail(fd)!=NULL)
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
                wcdma_error=true;
            }

        serialFlush(fd);
        serialClose(fd);

        return 0;
    }



    else if(state == 5)
    {
        QThread::sleep(1);

        while(serialDataAvail(fd)!=NULL)
        {
            system("sudo chmod 777 /dev/ttyAMA0");
            buf = serialGetchar(fd);
            rxbuffer[count]=buf;
            count++;
        }

        QString feed = QString(rxbuffer);

        qDebug()<<"debug : "<<rxbuffer;


        serialFlush(fd);
        serialClose(fd);

        return feed;
    }

    else if (state==6)
    {
        QThread::sleep(2);
        while(serialDataAvail(fd)!=NULL)
        {
            system("sudo chmod 777 /dev/ttyAMA0");
            buf = serialGetchar(fd);
            rxbuffer[count]=buf;
            count++;
        }

        QString feed = QString(rxbuffer);

        qDebug()<<"length : "<<feed.length();
        qDebug()<<"content : "<<feed;

        serialFlush(fd);
        serialClose(fd);

        return feed;
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
void MainWindow::send_append(char *TCPWRITE)
{
    char buffer[140]={0,};
    char ccs;

    bool error_flag = false;

    for(int i = 0;i<invCount;i++)
    {
        if(inv[i]->operatingStatus == 0x571)
            inv[i]->acPower=0;

        ccs = 0;
        ccs ^= 'T';
        ccs ^= (char)(plantNumber/0x100);
        ccs ^= (char)(plantNumber%0x100);
        ccs ^= (inv[i]->invID+0x31);

        //inv_number--;
        ccs ^= (char)0;
        ccs ^= (char)25;
        ccs ^= (char)0;
        ccs ^= (char)0;
        ccs ^= (char)0;
        ccs ^= (char)(inv[i]->operatingStatus/0x100);
        ccs ^= (char)(inv[i]->operatingStatus%0x100);
        ccs ^= (char)(0);
        ccs ^= (char)(0);
        ccs ^= (char)(0);
        ccs ^= (char)(0);
        ccs ^= (char)(0);
        ccs ^= (char)(0);

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
        ccs ^= (char)((inv[i]->acCurrent / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent / 1000)%0x100);

        ccs ^= (char)((inv[i]->acCurrent / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent / 1000)%0x100);
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
        ccs ^= (char)((inv[i]->acCurrent / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent / 1000)%0x100);

        ccs ^= (char)((inv[i]->acCurrent / 1000)/0x100);
        ccs ^= (char)((inv[i]->acCurrent / 1000)%0x100);
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
            (unsigned char)0,
            (unsigned char)25,
            (unsigned char)0,
            (unsigned char)0,
            (unsigned char)0,
            (unsigned char)(inv[i]->operatingStatus/0x100),
            (unsigned char)(inv[i]->operatingStatus%0x100),
            (unsigned char)(0),
            (unsigned char)(0),
            (unsigned char)(0),
            (unsigned char)(0),
            (unsigned char)(0),
            (unsigned char)(0),
            (unsigned char)((inv[i]->dcVoltage / 100)/0x100),
            (unsigned char)((inv[i]->dcVoltage / 100)%0x100),
            (unsigned char)((inv[i]->dcCurrent / 1000)/0x100),
            (unsigned char)((inv[i]->dcCurrent / 1000)%0x100),
            (unsigned char)((inv[i]->acVoltage1 / 100)/0x100),
            (unsigned char)((inv[i]->acVoltage1 / 100)%0x100),
            (unsigned char)((inv[i]->acVoltage2 / 100)/0x100),
            (unsigned char)((inv[i]->acVoltage2 / 100)%0x100),
            (unsigned char)((inv[i]->acVoltage3 / 100)/0x100),
            (unsigned char)((inv[i]->acVoltage3 / 100)%0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)/0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)%0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)/0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)%0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)/0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)%0x100),
            (unsigned char)((inv[i]->acFrequency/10)/0x100),
            (unsigned char)((inv[i]->acFrequency/10)%0x100),
            (unsigned char)((inv[i]->acVoltage1 / 100)/0x100),
            (unsigned char)((inv[i]->acVoltage1 / 100)%0x100),
            (unsigned char)((inv[i]->acVoltage2 / 100)/0x100),
            (unsigned char)((inv[i]->acVoltage2 / 100)%0x100),
            (unsigned char)((inv[i]->acVoltage3 / 100)/0x100),
            (unsigned char)((inv[i]->acVoltage3 / 100)%0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)/0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)%0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)/0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)%0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)/0x100),
            (unsigned char)((inv[i]->acCurrent / 1000)%0x100),
            (unsigned char)((inv[i]->acFrequency/10)/0x100),
            (unsigned char)((inv[i]->acFrequency/10)%0x100),
            (unsigned char)((inv[i]->dcPower / 100)/0x100),
            (unsigned char)((inv[i]->dcPower / 100)%0x100),
            (unsigned char)((inv[i]->totalYeild>>24)&0xff),
            (unsigned char)((inv[i]->totalYeild>>16)&0xff),
            (unsigned char)((inv[i]->totalYeild>>8)&0xff),
            (unsigned char)(inv[i]->totalYeild&0xff),
            (unsigned char)((inv[i]->acPower / 100)/0x100),
            (unsigned char)((inv[i]->acPower / 100)%0x100),
            (unsigned char)((inv[i]->acPower / 100)/0x100),
            (unsigned char)((inv[i]->acPower / 100)%0x100),
            (unsigned char)(inv[i]->dailyYeild/0x100),
            (unsigned char)(inv[i]->dailyYeild%0x100),
            (unsigned char)(0),
            (unsigned char)(0),
            ccs,
            0xaa
            );

        for(int i = 13 ; i<136+13;i++)
        {
            TCPWRITE[i] = buffer[i-13];
        }

        TCPWRITE[149] = 0x0d;
        TCPWRITE[150] = 0x0a;

        QString feed = uart_ch(TCPWRITE,5);

        if(feed.indexOf("OK") == -1)
        {
            error_flag |= true;
            break;
        }

    }

    if(error_flag == true)
    {
        wcdma_error=true;

        if(wcdma_count==5)
        {
            wcdma_count=0;
            wcdma_error=false;
        }

        wcdma_count++;
    }
    else
    {
        wcdma_error=false;
    }

}
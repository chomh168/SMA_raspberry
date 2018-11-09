#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "data.h"
#include "tcpclient.h"

#define INF 1


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //번호 가져오기


    ui->spinBox->setValue(getFileNum("port.txt"));
    ui->spinBox_2->setValue(getFileNum("inverter.txt"));

    plantNumber = ui->spinBox->value();
    invCount = ui->spinBox_2->value();

    //타이머 설정
    serTimer = new QTimer(this);
    serTimer->setInterval(600000);
    invTimer = new QTimer(this);
    invTimer->setInterval(10000);
    cheTimer = new QTimer(this);
    cheTimer->setInterval(1000);
    strTimer = new QTimer(this);
    strTimer->setInterval(5000);

    stpTimer = new QTimer(this);
    stpTimer->setInterval(600000);

    connect(serTimer,SIGNAL(timeout()),this,SLOT(serslot()));
    connect(invTimer,SIGNAL(timeout()),this,SLOT(invslot()));
    connect(cheTimer,SIGNAL(timeout()),this,SLOT(cheslot()));
    connect(strTimer,SIGNAL(timeout()),this,SLOT(strslot()));
    connect(stpTimer,SIGNAL(timeout()),this,SLOT(strslot()));

    connect(ui->pushButton_5, SIGNAL(clicked()),this,SLOT(invslot()));

    strTimer->start();
    ui->textBrowser->setText("5초후 start");



    //통신 설정
    for (int i = 0; i < 20; i++)
    {
         inv[i] = new Inverter(i);
    }

    connect(&watcher, SIGNAL(finished()), this, SLOT(csq_ok()));
    connect(&send_watcher, SIGNAL(finished()), this, SLOT(send_ok()));

    model = new QStandardItemModel(invCount,11,this); //

    model->setHorizontalHeaderItem(0, new QStandardItem(QString("status")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("DCV")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("DCA")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("DCKW")));
    model->setHorizontalHeaderItem(4, new QStandardItem(QString("ACV")));
    model->setHorizontalHeaderItem(5, new QStandardItem(QString("ACA")));
    model->setHorizontalHeaderItem(6, new QStandardItem(QString("ACKW")));
    model->setHorizontalHeaderItem(7, new QStandardItem(QString("PF")));
    model->setHorizontalHeaderItem(8, new QStandardItem(QString("daytotal")));
    model->setHorizontalHeaderItem(9, new QStandardItem(QString("total")));
    model->setHorizontalHeaderItem(10, new QStandardItem(QString("datetime")));

    ui->tableView->setModel(model);

    wiringPiSetup();
    pinMode(1,INPUT);

    QFuture<QString> th1 = QtConcurrent::run(MainWindow::req_csq);
    watcher.setFuture(th1);



}




MainWindow::~MainWindow()
{
    delete ui;
}


//클릭 이벤트(port)
void MainWindow::on_pushButton_clicked()
{
    plantNumber = ui->spinBox->value();
    setFileNum("port.txt",plantNumber);

}

//클릭 이벤트(inverter)
void MainWindow::on_pushButton_2_clicked()
{
    invCount = ui->spinBox_2->value();
    setFileNum("inverter.txt",invCount);
    qDebug()<<invCount;

    model = new QStandardItemModel(invCount,11,this); //

    model->setHorizontalHeaderItem(0, new QStandardItem(QString("status")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("DCV")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("DCA")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("DCKW")));
    model->setHorizontalHeaderItem(4, new QStandardItem(QString("ACV")));
    model->setHorizontalHeaderItem(5, new QStandardItem(QString("ACA")));
    model->setHorizontalHeaderItem(6, new QStandardItem(QString("ACKW")));
    model->setHorizontalHeaderItem(7, new QStandardItem(QString("PF")));
    model->setHorizontalHeaderItem(8, new QStandardItem(QString("daytotal")));
    model->setHorizontalHeaderItem(9, new QStandardItem(QString("total")));
    model->setHorizontalHeaderItem(10, new QStandardItem(QString("datetime")));

    ui->tableView->setModel(model);
}

//번호 받기
int MainWindow::getFileNum(QString str)
{
    QString filename = str;
    QFile File(QApplication::applicationDirPath() + "/" + filename);

    if(!File.open(QFile::ReadOnly|QFile::Text)) // 읽기 전용, 텍스트로 파일 열기
    {
         if(!File.exists()) // 파일이 존재하지 않으면...
        {
             if(str=="port.txt")
                return 7777;
             else
                 return 1;
        }

    }

    QTextStream OpenFile(&File);
    QString ConfigText;
    while(!OpenFile.atEnd())  // 파일 끝까지 읽어서
    {
         ConfigText=OpenFile.readLine(); // 한라인씩 읽어서 변수에 적용
    }
    File.close(); // 파일닫기

    return ConfigText.toInt();
}

//번호 입력
void MainWindow::setFileNum(QString str, int Num)
{
    QFile *file = new QFile;

    file->setFileName(QApplication::applicationDirPath() + "/" + str); //파일의 위치와 파일명 설정

    if (!file->open(QIODevice::WriteOnly)) //파일을 오픈
    {
        qDebug() << "Error: File Not open";
    }

    QString strNum(QString::number(Num)); //파일에 작성할 테스트
    file->write(strNum.toUtf8()); //파일에 텍스트를 작성
    file->close();
}

void MainWindow::setFileLog(QString log)
{
    QFile *file = new QFile;
    QString str = "log.txt";

    file->setFileName(QApplication::applicationDirPath() + "/" + str); //파일의 위치와 파일명 설정

    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) //파일을 오픈
    {
        qDebug() << "Error: File Not open";
    }

    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    file->write(time.toUtf8()+" " + log.toUtf8() + "\r\n"); //파일에 텍스트를 작성
    file->close();
}



void MainWindow::on_pushButton_3_clicked()
{
   invTimer->start();
   serTimer->start();
   cheTimer->start();

   ui->textBrowser->clear();
}
void MainWindow::on_pushButton_4_clicked()
{
    invTimer->stop();
    serTimer->stop();
    cheTimer->stop();
    strTimer->stop();
    ui->textBrowser->append("stop");

    stpTimer->start();
}


void MainWindow::serslot()
{
    if(toggle==true)
    {
        QFuture<void> th2 = QtConcurrent::run(MainWindow::SendWCDMA);
        send_watcher.setFuture(th2);
    }
    else
    {
        SendServerHstec();
    }

}

void MainWindow::invslot()
{

    if(selectSendMsgType == 1)
    {
        for(int i = 0; i<invCount;i++)
        {
            TSEND[selectSendMsgType-1][1] = (char) i;
            QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType == 2)
    {
        for(int i = 0; i<invCount;i++)
        {
            TSEND[selectSendMsgType-1][1] = (char) i;
            QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType == 3)
    {
        for(int i = 0; i<invCount;i++)
        {

            TSEND[selectSendMsgType-1][1] = (char) i;
            QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);

        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType ==4)
    {
        for(int i = 0; i<invCount;i++)
        {
            TSEND[selectSendMsgType-1][1] = (char) i;
            QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);

            QStandardItem *Item = new QStandardItem(QString::number(inv[i]->totalYeild));
            model->setItem(i,9,Item);
            Item = new QStandardItem(QString::number(inv[i]->dailyYeild));
            model->setItem(i,8,Item);
            Item = new QStandardItem(QString::number(inv[i]->acCurrent / 1000));
            model->setItem(i,5,Item);
            Item = new QStandardItem(QString::number(inv[i]->acPower / 1000));
            model->setItem(i,6,Item);
            Item = new QStandardItem(QString::number(inv[i]->acVoltage1 / 100));
            model->setItem(i,4,Item);

            QString state="Null";
            if(inv[i]->operatingStatus == 0x127) state = "발전";
            else if(inv[i]->operatingStatus == 0x571) state = "정지";
            else if(inv[i]->operatingStatus == 0x5BB) state = "준비";
            else if(first==false)
            {
               state = "ERROR("+QString::number(inv[i]->operatingStatus)+")";
               setFileLog(invIP[i]+" "+state);
            }
            Item = new QStandardItem(state);
            model->setItem(i,0,Item);

            Item = new QStandardItem(QString::number(inv[i]->dcCurrent / 1000));
            model->setItem(i,2,Item);
            Item = new QStandardItem(QString::number(inv[i]->dcVoltage / 100));
            model->setItem(i,1,Item);
            Item = new QStandardItem(QString::number(inv[i]->dcPower / 1000));
            model->setItem(i,3,Item);
            Item = new QStandardItem(QString::number(0));
            model->setItem(i,7,Item);
            QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            Item = new QStandardItem(time);
            model->setItem(i,10,Item);

            ui->tableView->setModel(model);
            ui->tableView->resizeColumnsToContents();
            ui->tableView->resizeRowsToContents();
        }
        selectSendMsgType=1;

        if(first==true)
        {
            QFuture<void> th5 = QtConcurrent::run(MainWindow::SendWCDMA);
            send_watcher.setFuture(th5);
        }

        first=false;


    }

    //QtConcurrent::run(MainWindow::SMSReceive);



}

void MainWindow::SMSReceive()
{
    //QThread::sleep(2);

    char CMGR[12] = {0x41, 0x54 , 0x2B , 0x43 , 0x4D , 0x47 , 0x52 , 0x3D , 0x30 , 0x0D};

    char CNUM[10] = {0x41, 0x54 , 0x24 , 0x24 , 0x43 , 0x4e , 0x55 , 0x4d , 0x0d};

    char CMGD[12] = {0x41, 0x54, 0x2B, 0x43, 0x4D, 0x47, 0x44, 0x3D, 0x30, 0x0D};

    char SMSW[60] = {0x41, 0x54 , 0x24 , 0x24, 0x53 , 0x4D , 0x53 , 0x57 , 0x3D , 0x22 , 0x30 , 0x31 , 0x30 , 0x35 , 0x35 , 0x37 , 0x37 , 0x37 , 0x36 , 0x32 , 0x37 , 0x22 , 0x2C , 0x22 , 0x30 , 0x31 , 0x32 , 0x32 , 0x39 , 0x31 , 0x38 , 0x36 , 0x36 , 0x32 , 0x32 , 0x22 , 0x2C , 0x30 , 0x2C , 0x30 , 0x2C , 0x30 , 0x2C , 0x31 , 0x33 , 0x32 , 0x0D};

    char MSG[6] = {0x4F, 0x4B, 0x1A, 0x0D};

    char rxbuffer[256] = {0,};
    int fd;
    int count=0;
    char buf;

        if((fd=serialOpen("/dev/serial0",115200))<0)
        {
            qDebug()<<"err:not Open";
        }
        serialFlush(fd);

    serialPuts(fd,CNUM);

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

        qDebug()<<str;

        if(str.indexOf("8212")>0)
        {
            for(int i = 0;i<8;i++)
            {
                SMSW[27+i] = str.at(str.indexOf("8212")+4+i).toLatin1();
            }
        }

        serialPuts(fd,CMGR);
        qDebug()<<SMSW;

    QThread::sleep(2);

        while(serialDataAvail(fd)!=NULL)
        {
            system("sudo chmod 777 /dev/ttyAMA0");
            buf = serialGetchar(fd);
            rxbuffer[count]=buf;
            count++;
        }

        QString feed = QString(rxbuffer);
            qDebug()<<feed;

    if(feed.indexOf("010")>0)
    {
        for(int i =0;i<11;i++)
        {
            SMSW[10+i] = feed.at(feed.indexOf("010")+i).toLatin1();
        }

    }

    if(feed.indexOf("5339383230")>0)
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
        qDebug()<<rxbuffer;

        serialPuts(fd,MSG);
    }

    QThread::sleep(1);

    serialPuts(fd,CMGD);

    QThread::sleep(1);

    serialFlush(fd);
    serialClose(fd);

}

void MainWindow::cheslot()
{
    ui->label_3->setText("server  : "+QString::number(serTimer->remainingTime()/1000));//
    ui->label_4->setText("inverter : "+QString::number(invTimer->remainingTime()/1000));//

    if(digitalRead(1)==1)
    {
        char buffer[255];
        sprintf(buffer,"python /home/pi/Desktop/shapes.py %d %d %d %d %d",plantNumber,jj+1,NCSQ,inv[jj]->acCurrent,inv[jj]->dailyYeild);
        system(buffer);
        jj++;
        jj = ((jj)%invCount);

        QFuture<QString> th1 = QtConcurrent::run(MainWindow::req_csq);
        watcher.setFuture(th1);
    }
}
void MainWindow::strslot()
{
    invTimer->start();
    serTimer->start();
    cheTimer->start();


    strTimer->stop();
    ui->textBrowser->clear();
}


bool MainWindow::SendMessage(QString ipaddress, int selectSendMsgType, int count)
{

    TcpClient *client = new TcpClient();
    bool check = client->TcpConnect(ipaddress,502);

    if(check==true)
    {
        client->onConnectServer();
        client->sendRequst(TSEND[selectSendMsgType-1],sizeof(TSEND[selectSendMsgType]));


        client->readMessage();

        if (selectSendMsgType == 1)
        {
            inv[count]->totalYeild = (client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12));
            inv[count]->dailyYeild = (client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24));
        }
        else if (selectSendMsgType == 2)
        {
            inv[count]->dcCurrentA = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
            inv[count]->dcVoltageA = client->getBuf(13) * 0x1000000 + client->getBuf(14) * 0x10000 + client->getBuf(15) * 0x100 + client->getBuf(16);
            inv[count]->dcPowerA = client->getBuf(17) * 0x1000000 + client->getBuf(18) * 0x10000 + client->getBuf(19) * 0x100 + client->getBuf(20);
            inv[count]->acPower = client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24);
            inv[count]->acVoltage1 = client->getBuf(39) * 0x100 + client->getBuf(40) == 65535 ? 0 : (int)((client->getBuf(39) * 0x100 + client->getBuf(40)) * qSqrt(3));
            inv[count]->acVoltage2 = client->getBuf(43) * 0x100 + client->getBuf(44) == 65535 ? 0 : (int)((client->getBuf(43) * 0x100 + client->getBuf(44)) * qSqrt(3));
            inv[count]->acVoltage3 = client->getBuf(47) * 0x100 + client->getBuf(48) == 65535 ? 0 : (int)((client->getBuf(47) * 0x100 + client->getBuf(48)) * qSqrt(3));
            inv[count]->acCurrent = client->getBuf(61) * 0x1000000 + client->getBuf(62) * 0x10000 + client->getBuf(63) * 0x100 + client->getBuf(64) == 65535 ? 0 : client->getBuf(61) * 0x1000000 + client->getBuf(62) * 0x10000 + client->getBuf(63) * 0x100 + client->getBuf(64);
            if(inv[count]->acCurrent==2123) inv[count]->acCurrent = 0;
            inv[count]->acFrequency = client->getBuf(77) * 0x1000000 + client->getBuf(78) * 0x10000 + client->getBuf(79) * 0x100 + client->getBuf(80) == 65535 ? 0 : client->getBuf(77) * 0x1000000 + client->getBuf(78) * 0x10000 + client->getBuf(79) * 0x100 + client->getBuf(80);

        }
        else if (selectSendMsgType == 3)
        {

            inv[count]->dcCurrentB = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
            inv[count]->dcVoltageB = client->getBuf(13) * 0x1000000 + client->getBuf(14) * 0x10000 + client->getBuf(15) * 0x100 + client->getBuf(16);
            inv[count]->dcPowerB = client->getBuf(17) * 0x1000000 + client->getBuf(18) * 0x10000 + client->getBuf(19) * 0x100 + client->getBuf(20);

            inv[count]->dcCurrent = (inv[count]->dcCurrentA + inv[count]->dcCurrentB);
            inv[count]->dcVoltage = (inv[count]->dcVoltageA + inv[count]->dcVoltageB) / 2;
            inv[count]->dcPower = (inv[count]->dcPowerA + inv[count]->dcPowerB);

        }
        else if (selectSendMsgType == 4)
        {
            inv[count]->operatingStatus = client->getBuf(11) * 0x100 + client->getBuf(12);

        }

        client->TcpDisconnect();
    }


    return check;
}



void addPacket(int cnt, int value)
{
    sdata[cnt] = (char)(value&0xff);
    checksum ^= sdata[cnt];
}

//WCDMA를 통한 전송
void MainWindow::SendWCDMA()
{
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
        //bool reboot=false;

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
                            //setFileLog("reboot");
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

                if(reboot==true)
                {
                    //setFileLog("reboot");
                    //system("reboot");
                }

            }
        }
    }
}

QString MainWindow::uart_ch(char *ch, int state)
{
    char rxbuffer[100] = {0,};
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

void MainWindow::send_append(char *TCPWRITE)
{
    char buffer[140]={0,};
    char ccs;

    bool error_flag = false;

    for(int i = 0;i<invCount;i++)
    {
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

    }



    for (int i = 0; i < invCount; i++)
    {
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
        addPacket(9, 0);        // blackout im_key
        addPacket(10, inv[i]->operatingStatus / 0x100);    // inverter_status
        addPacket(11, inv[i]->operatingStatus % 0x100);    // inverter_status
        addPacket(12, 0);    // inverter_status
        addPacket(13, 0);    // inverter_status
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
        //SendMessageHstec("220.122.203.111",7778,sdata,sizeof(sdata));
     }
}

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



void MainWindow::on_pushButton_5_clicked()
{
    invslot();
}

void MainWindow::on_pushButton_6_clicked()
{
    serslot();
}

void MainWindow::on_pushButton_7_clicked()
{


    QString str = "leafpad "+QApplication::applicationDirPath()+"/log.txt";
    qDebug()<<str;

    system(str.toUtf8());
}

QString MainWindow::req_csq()
{
    system("sudo chmod 777 /dev/ttyAMA0");
    char ATE[7] = {0x41, 0x54, 0x45, 0x30, 0x0D, 0x0A};
    char CSQ[8] = {0x41, 0x54, 0x2B, 0x43, 0x53, 0x51, 0x0D};
    QString str;

    uart_ch(ATE,0);

    unsigned int nextTime;
    nextTime = millis () + 30000;


    while(INF)
    {
        str = uart_ch(CSQ,1);

        if(millis() > nextTime)
            return "ERROR";
        if(str.length()>0)
            break;
    }

    //NCSQ = str[8].digitValue()*10 + str[9].digitValue();

    return str;
}

void MainWindow::csq_ok()
{
    csq = watcher.result();
    if(csq.length()==7)
        NCSQ=0;
    else
        NCSQ = csq[8].digitValue()*10 + csq[9].digitValue();

    ui->label_5->setText("CSQ : "+QString::number(NCSQ));
    QMessageBox::information(this,"info","CSQ : "+QString::number(NCSQ),"OK");
}

void MainWindow::send_ok()
{
    ui->textBrowser->clear();
    ui->textBrowser->append("WCDMA send ok!!");

    ui->label_5->setText("CSQ : "+QString::number(NCSQ));

    if(wcdma_error==true)
    {
        setFileLog("wcdma error");
        QFuture<void> th5 = QtConcurrent::run(MainWindow::SendWCDMA);
        send_watcher.setFuture(th5);
    }
    else
    {
        QtConcurrent::run(MainWindow::SMSReceive);
    }
    if(reboot==true)
    {
        setFileLog("reboot");
        system("reboot");
    }

}

void MainWindow::on_pushButton_8_clicked()
{
    QFuture<QString> th1 = QtConcurrent::run(MainWindow::req_csq);
    watcher.setFuture(th1);

}

void MainWindow::on_pushButton_9_clicked()
{
    if(toggle==true)
    {
        toggle=false;
        ui->pushButton_9->setText("LAN");
    }
    else
    {
        toggle=true;
        ui->pushButton_9->setText("WCD");
    }
}

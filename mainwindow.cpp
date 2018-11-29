#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "data.h"
#include "tcpclient.h"

#define INF 1



Inverter* inv[20]; //= new Inverter[20];


//25K STP
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

//시작 버튼
void MainWindow::on_pushButton_3_clicked()
{
   invTimer->start();
   serTimer->start();
   cheTimer->start();

   ui->textBrowser->clear();
}

//정지 버튼
void MainWindow::on_pushButton_4_clicked()
{
    invTimer->stop();
    serTimer->stop();
    cheTimer->stop();
    strTimer->stop();
    ui->textBrowser->append("stop");

    stpTimer->start();
}

//인버터 요청
void MainWindow::on_pushButton_5_clicked()
{
    invslot();
}

//인버터 요청
void MainWindow::invslot()
{
    //25K STP
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

        if(send_watcher.isRunning()==false)
        {
            //QFuture<void> th6 = QtConcurrent::run(MainWindow::SMSReceive);
            //sms_watcher.setFuture(th6);
        }
    }
}

//메세지를 담는 부분
bool MainWindow::SendMessage(QString ipaddress, int selectSendMsgType, int count)
{
    //25K STP
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


//서버 전송
void MainWindow::on_pushButton_6_clicked()
{
    serslot();
}

//서버 전송
void MainWindow::serslot()
{
    if(toggle==true)
    {
        while(sms_watcher.isRunning()==true) QThread::sleep(20);

        QFuture<void> th2 = QtConcurrent::run(MainWindow::SendWCDMA);
        send_watcher.setFuture(th2);
    }
    else
    {
        SendServerHstec();
    }

}

//로그 보기
void MainWindow::on_pushButton_7_clicked()
{
    QString str = "leafpad "+QApplication::applicationDirPath()+"/log.txt";
    qDebug()<<str;

    system(str.toUtf8());
}

//CSQ 요청
void MainWindow::on_pushButton_8_clicked()
{
    QFuture<QString> th1 = QtConcurrent::run(MainWindow::req_csq);
    watcher.setFuture(th1);

}

//통신 방법 교환
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

//1초 마다 확인
void MainWindow::cheslot()
{
    ui->label_3->setText("server  : "+QString::number(serTimer->remainingTime()/1000));//
    ui->label_4->setText("inverter : "+QString::number(invTimer->remainingTime()/1000));//

    if(digitalRead(1)==1)
    {
        char cmd[255];
        sprintf(cmd,"python /home/pi/Desktop/shapes.py %d %d %d %d %d",plantNumber,jj+1,NCSQ,inv[jj]->acCurrent,inv[jj]->dailyYeild);
        system(cmd);
        jj++;
        jj = ((jj)%invCount);

        QFuture<QString> th1 = QtConcurrent::run(MainWindow::req_csq);
        watcher.setFuture(th1);
    }
}

//초기 시작
void MainWindow::strslot()
{
    invTimer->start();
    serTimer->start();
    cheTimer->start();


    strTimer->stop();
    ui->textBrowser->clear();
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

//csq 확인
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

//csq 확인
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

//wcdma 전송 확인
void MainWindow::send_ok()
{
    ui->textBrowser->clear();
    ui->textBrowser->append("WCDMA send ok!!");

    ui->label_5->setText("CSQ : "+QString::number(NCSQ));

    if(wcdma_error==true)
    {
        while(sms_watcher.isRunning()==true) QThread::sleep(20);

        setFileLog("wcdma error");
        QFuture<void> th5 = QtConcurrent::run(MainWindow::SendWCDMA);
        send_watcher.setFuture(th5);
    }
    else
    {

        //QFuture<void> th6 = QtConcurrent::run(MainWindow::SMSReceive);
        //sms_watcher.setFuture(th6);
    }

    if(reboot==true)
    {
        setFileLog("reboot");
        system("reboot");
    }

}


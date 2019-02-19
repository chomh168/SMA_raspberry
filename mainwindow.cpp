#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "data.h"
#include "tcpclient.h"

#define INF 1

enum{

    STP25K=0,
    STP50K,
    SC1M
};


Inverter* inv[20]; //= new Inverter[20];



char sdata[70] = {0,};// = new char[70];    // 구서버 송신 버퍼
char checksum = 0;              // 구서버 송신 체크섬


int plantNumber=7777;
int invCount=1;

bool first=true;
bool toggle=true;

bool wcdma_error=false;
int wcdma_count=0;

bool reboot=false;
int NCSQ=0;

int capacity = 0;

int error_count=0;
bool send_error=false;

bool black=false;
bool count_error=false;

int check_count=0;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //번호 가져오기
    ui->spinBox->setValue(getFileNum("port.txt"));
    ui->spinBox_2->setValue(getFileNum("inverter.txt"));

    //초기 번호 입력
    plantNumber = ui->spinBox->value();
    invCount = ui->spinBox_2->value();

    //타이머 설정
    serTimer = new QTimer(this);
    serTimer->setInterval(300000);
    invTimer = new QTimer(this);
    invTimer->setInterval(10000);
    cheTimer = new QTimer(this);
    cheTimer->setInterval(1000);
    strTimer = new QTimer(this);
    strTimer->setInterval(5000);

    stpTimer = new QTimer(this);
    stpTimer->setInterval(600000);

    //타임 아웃 설정
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

    //테이블 설정
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

    wiringPiSetup(); //gpio 설정

    pinMode(1,INPUT); // 외부 스위치 입력

    pinMode(7,INPUT); // 정전 감지 센서

    pinMode(0,OUTPUT);
    digitalWrite(0,0); //리셋 신호(HW)


    QFuture<QString> th1 = QtConcurrent::run(MainWindow::req_csq);
    watcher.setFuture(th1);

    ui->comboBox->addItem("25K");
    ui->comboBox->addItem("50K");
    ui->comboBox->addItem("1M");

    capacity = getFileNum("capacity.txt");
    ui->comboBox->setCurrentIndex(capacity);

    toggle = getFileNum("mode.txt");
    if(toggle==true)
        ui->pushButton_9->setText("WCD");
    else
        ui->pushButton_9->setText("LAN");

    connect(ui->actionHW_reset_2, SIGNAL(triggered()),this,SLOT(hw_reset()));
    connect(ui->actionSW_reset_2, SIGNAL(triggered()),this,SLOT(sw_reset()));
}


MainWindow::~MainWindow()
{
    delete ui;
}



//인버터 요청
void MainWindow::invslot()
{
    //25K
    if(capacity==STP25K)
    {
        inv25K();
        /*
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
                ui->textBrowser->clear();
                ui->textBrowser->append("전송중..");

                if(toggle==true){
                    QFuture<void> th5 = QtConcurrent::run(MainWindow::SendWCDMA);
                    send_watcher.setFuture(th5);
                }
                else{
                    SendServerHstec();
                }

                first=false;
            }




        }

        */
    }

    //50K
    else if(capacity==STP50K)
    {
        inv50K();
        /*
        //50K STP
        if(selectSendMsgType == 1)
        {
            for(int i = 0; i<invCount;i++)
            {
                T5SEND[selectSendMsgType-1][1] = (char) i;
                QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);
            }
            selectSendMsgType++;
        }
        else if(selectSendMsgType == 2)
        {
            for(int i = 0; i<invCount;i++)
            {
                T5SEND[selectSendMsgType-1][1] = (char) i;
                QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);
            }
            selectSendMsgType++;
        }
        else if(selectSendMsgType == 3)
        {
            for(int i = 0; i<invCount;i++)
            {

                T5SEND[selectSendMsgType-1][1] = (char) i;
                QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);

            }
            selectSendMsgType++;
        }
        else if(selectSendMsgType ==4)
        {
            for(int i = 0; i<invCount;i++)
            {
                T5SEND[selectSendMsgType-1][1] = (char) i;
                QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);

                QStandardItem *Item = new QStandardItem(QString::number(inv[i]->totalYeild));
                model->setItem(i,9,Item);
                Item = new QStandardItem("-");
                model->setItem(i,8,Item);
                Item = new QStandardItem(QString::number(inv[i]->acCurrent / 1000));
                model->setItem(i,5,Item);
                Item = new QStandardItem(QString::number(inv[i]->acPower / 1000));
                model->setItem(i,6,Item);
                Item = new QStandardItem(QString::number(inv[i]->acVoltage1 / 100));
                model->setItem(i,4,Item);

                QString state="Null";
                if(inv[i]->operatingStatus == 0x127) state = "발전";
                else if(inv[i]->operatingStatus == 0xFFFD) state = "정지";
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
            selectSendMsgType++;

            if(first==true)
            {
                ui->textBrowser->clear();
                ui->textBrowser->append("전송중..");

                if(toggle==true){
                    QFuture<void> th5 = QtConcurrent::run(MainWindow::SendWCDMA);
                    send_watcher.setFuture(th5);
                }
                else{
                    SendServerHstec();
                }

                first=false;
            }



        }
        else if(selectSendMsgType == 5)
        {
            for(int i = 0; i<invCount;i++)
            {
                T5SEND[selectSendMsgType-1][1] = (char) i;
                QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);
            }
            selectSendMsgType=1;
        }
        */
    }

    //1M
    else if(capacity==SC1M)
    {
        inv1M();
        /*
        if(selectSendMsgType == 1)
        {
            for(int i = 0; i<invCount;i++)
            {
                C1SEND[selectSendMsgType-1][1] = (char) i;
                QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);
            }
            selectSendMsgType++;
        }
        else if(selectSendMsgType == 2)
        {
            for(int i = 0; i<invCount;i++)
            {
                C1SEND[selectSendMsgType-1][1] = (char) i;
                QtConcurrent::run(MainWindow::SendMessage,invIP[i],selectSendMsgType,i);
            }
            selectSendMsgType++;
        }
        else if(selectSendMsgType ==3)
        {
            for(int i = 0; i<invCount;i++)
            {
                C1SEND[selectSendMsgType-1][1] = (char) i;
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
                if(inv[i]->operatingStatus == 0x135) state = "발전";
                else if(inv[i]->operatingStatus == 0x571) state = "정지";
                //else if(inv[i]->operatingStatus == 0x5BB) state = "준비";
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
                ui->textBrowser->clear();
                ui->textBrowser->append("전송중..");

                if(toggle==true){
                    QFuture<void> th5 = QtConcurrent::run(MainWindow::SendWCDMA);
                    send_watcher.setFuture(th5);
                }
                else{
                    SendServerHstec();
                }

                first=false;
            }

        }*/
    }
}

//메세지를 담는 부분
bool MainWindow::SendMessage(QString ipAddress, int selectSendMsgType, int index)
{

    TcpClient *client = new TcpClient();
    bool check=true;
    //bool check = client->TcpConnect(ipAddress,502);


    if(check==true)
    {
        //25K STP
        if(capacity==STP25K)
        {
            return SendMessage25K(ipAddress,selectSendMsgType,index);
            /*
            client->onConnectServer();
            client->sendRequst(TSEND[selectSendMsgType-1],sizeof(TSEND[selectSendMsgType-1]));


            client->readMessage();

            if (selectSendMsgType == 1)
            {
                inv[index]->totalYeild = (client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12));
                inv[index]->dailyYeild = (client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24));
            }
            else if (selectSendMsgType == 2)
            {
                inv[index]->dcCurrentA = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
                inv[index]->dcVoltageA = client->getBuf(13) * 0x1000000 + client->getBuf(14) * 0x10000 + client->getBuf(15) * 0x100 + client->getBuf(16);
                inv[index]->dcPowerA = client->getBuf(17) * 0x1000000 + client->getBuf(18) * 0x10000 + client->getBuf(19) * 0x100 + client->getBuf(20);
                inv[index]->acPower = client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24);
                inv[index]->acVoltage1 = client->getBuf(39) * 0x100 + client->getBuf(40) == 65535 ? 0 : (int)((client->getBuf(39) * 0x100 + client->getBuf(40)) * qSqrt(3));
                inv[index]->acVoltage2 = client->getBuf(43) * 0x100 + client->getBuf(44) == 65535 ? 0 : (int)((client->getBuf(43) * 0x100 + client->getBuf(44)) * qSqrt(3));
                inv[index]->acVoltage3 = client->getBuf(47) * 0x100 + client->getBuf(48) == 65535 ? 0 : (int)((client->getBuf(47) * 0x100 + client->getBuf(48)) * qSqrt(3));
                inv[index]->acCurrent = client->getBuf(61) * 0x1000000 + client->getBuf(62) * 0x10000 + client->getBuf(63) * 0x100 + client->getBuf(64) == 65535 ? 0 : client->getBuf(61) * 0x1000000 + client->getBuf(62) * 0x10000 + client->getBuf(63) * 0x100 + client->getBuf(64);
                if(inv[index]->acCurrent==2123) inv[index]->acCurrent = 0;
                inv[index]->acFrequency = client->getBuf(77) * 0x1000000 + client->getBuf(78) * 0x10000 + client->getBuf(79) * 0x100 + client->getBuf(80) == 65535 ? 0 : client->getBuf(77) * 0x1000000 + client->getBuf(78) * 0x10000 + client->getBuf(79) * 0x100 + client->getBuf(80);

            }
            else if (selectSendMsgType == 3)
            {

                inv[index]->dcCurrentB = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
                inv[index]->dcVoltageB = client->getBuf(13) * 0x1000000 + client->getBuf(14) * 0x10000 + client->getBuf(15) * 0x100 + client->getBuf(16);
                inv[index]->dcPowerB = client->getBuf(17) * 0x1000000 + client->getBuf(18) * 0x10000 + client->getBuf(19) * 0x100 + client->getBuf(20);

                inv[index]->dcCurrent = (inv[index]->dcCurrentA + inv[index]->dcCurrentB);
                inv[index]->dcVoltage = (inv[index]->dcVoltageA + inv[index]->dcVoltageB) / 2;
                inv[index]->dcPower = (inv[index]->dcPowerA + inv[index]->dcPowerB);

            }
            else if (selectSendMsgType == 4)
            {
                inv[index]->operatingStatus = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);

            }

            client->TcpDisconnect();
            */
        }

        //50K STP
        else if(capacity==STP50K)
        {
            return SendMessage50K(ipAddress,selectSendMsgType,index);

            /*
            client->onConnectServer();
            client->sendRequst(T5SEND[selectSendMsgType-1],sizeof(T5SEND[selectSendMsgType-1]));


            client->readMessage();

            if (selectSendMsgType == 1)
            {
                inv[index]->totalYeild = (client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12));
                inv[index]->dailyYeild = (client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24));
            }
            else if (selectSendMsgType == 2)
            {
                inv[index]->dcCurrentA = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
                inv[index]->dcVoltageA = client->getBuf(13) * 0x1000000 + client->getBuf(14) * 0x10000 + client->getBuf(15) * 0x100 + client->getBuf(16);
                inv[index]->dcPowerA = client->getBuf(17) * 0x1000000 + client->getBuf(18) * 0x10000 + client->getBuf(19) * 0x100 + client->getBuf(20);
                inv[index]->acPower = client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24);
                inv[index]->acVoltage1 = client->getBuf(39) * 0x100 + client->getBuf(40) == 65535 ? 0 : (int)((client->getBuf(39) * 0x100 + client->getBuf(40)) * qSqrt(3));
                inv[index]->acVoltage2 = client->getBuf(43) * 0x100 + client->getBuf(44) == 65535 ? 0 : (int)((client->getBuf(43) * 0x100 + client->getBuf(44)) * qSqrt(3));
                inv[index]->acVoltage3 = client->getBuf(47) * 0x100 + client->getBuf(48) == 65535 ? 0 : (int)((client->getBuf(47) * 0x100 + client->getBuf(48)) * qSqrt(3));
                inv[index]->acCurrent = client->getBuf(61) * 0x1000000 + client->getBuf(62) * 0x10000 + client->getBuf(63) * 0x100 + client->getBuf(64) == 65535 ? 0 : client->getBuf(61) * 0x1000000 + client->getBuf(62) * 0x10000 + client->getBuf(63) * 0x100 + client->getBuf(64);
                inv[index]->acFrequency = client->getBuf(77) * 0x1000000 + client->getBuf(78) * 0x10000 + client->getBuf(79) * 0x100 + client->getBuf(80) == 65535 ? 0 : client->getBuf(77) * 0x1000000 + client->getBuf(78) * 0x10000 + client->getBuf(79) * 0x100 + client->getBuf(80);
                if (inv[index]->operatingStatus == 0xFFFD)
                {
                    inv[index]->acPower = 0;
                }
            }
            else if (selectSendMsgType == 3)
            {

                inv[index]->dcCurrentB = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
                inv[index]->dcVoltageB = client->getBuf(13) * 0x1000000 + client->getBuf(14) * 0x10000 + client->getBuf(15) * 0x100 + client->getBuf(16);
                inv[index]->dcPowerB = client->getBuf(17) * 0x1000000 + client->getBuf(18) * 0x10000 + client->getBuf(19) * 0x100 + client->getBuf(20);

                inv[index]->dcCurrentC = client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24);
                inv[index]->dcPowerC = client->getBuf(29) * 0x1000000 + client->getBuf(30) * 0x10000 + client->getBuf(31) * 0x100 + client->getBuf(32);
                inv[index]->dcCurrentD = client->getBuf(33) * 0x1000000 + client->getBuf(34) * 0x10000 + client->getBuf(35) * 0x100 + client->getBuf(36);
                inv[index]->dcPowerD = client->getBuf(41) * 0x1000000 + client->getBuf(42) * 0x10000 + client->getBuf(43) * 0x100 + client->getBuf(44);


                inv[index]->dcVoltage = (inv[index]->dcVoltageA + inv[index]->dcVoltageB) / 2;


            }
            else if (selectSendMsgType == 4)
            {
                inv[index]->operatingStatus = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);

            }

            else if (selectSendMsgType == 5)
            {
                inv[index]->dcCurrentE = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
                inv[index]->dcPowerE = client->getBuf(17) * 0x1000000 + client->getBuf(18) * 0x10000 + client->getBuf(19) * 0x100 + client->getBuf(20);
                inv[index]->dcCurrentF = client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24);
                inv[index]->dcPowerF = client->getBuf(29) * 0x1000000 + client->getBuf(30) * 0x10000 + client->getBuf(31) * 0x100 + client->getBuf(32);

                inv[index]->dcCurrent = (inv[index]->dcCurrentA + inv[index]->dcCurrentB + inv[index]->dcCurrentC + inv[index]->dcCurrentD + inv[index]->dcCurrentE
                                         + inv[index]->dcCurrentF);
                inv[index]->dcVoltage = (inv[index]->dcVoltageA + inv[index]->dcVoltageB) / 2;
                inv[index]->dcPower = (inv[index]->dcPowerA + inv[index]->dcPowerB + inv[index]->dcPowerC + inv[index]->dcPowerD + inv[index]->dcPowerE
                                        + inv[index]->dcPowerF);

            }

            client->TcpDisconnect();
            */
        }

        //1M SC
        else if(capacity==SC1M)
        {
            return SendMessage1M(ipAddress,selectSendMsgType,index);

            /*
            client->onConnectServer();
            client->sendRequst(C1SEND[selectSendMsgType-1],sizeof(C1SEND[selectSendMsgType-1]));


            client->readMessage();

            if (selectSendMsgType == 1)
            {
                inv[index]->totalYeild = (client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12));
                inv[index]->dailyYeild = (client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24));
            }
            else if (selectSendMsgType == 2)
            {
                inv[index]->dcCurrent = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
                inv[index]->dcVoltage = client->getBuf(13) * 0x1000000 + client->getBuf(14) * 0x10000 + client->getBuf(15) * 0x100 + client->getBuf(16);
                inv[index]->dcPower = client->getBuf(17) * 0x1000000 + client->getBuf(18) * 0x10000 + client->getBuf(19) * 0x100 + client->getBuf(20);
                inv[index]->acPower = client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24);
                inv[index]->acVoltage1 = client->getBuf(51) * 0x100 + client->getBuf(52) == 65535 ? 0 : (int)((client->getBuf(51) * 0x100 + client->getBuf(52)));// * qSqrt(3));
                inv[index]->acVoltage2 = client->getBuf(55) * 0x100 + client->getBuf(56) == 65535 ? 0 : (int)((client->getBuf(55) * 0x100 + client->getBuf(56)));// * qSqrt(3));
                inv[index]->acVoltage3 = client->getBuf(59) * 0x100 + client->getBuf(60) == 65535 ? 0 : (int)((client->getBuf(59) * 0x100 + client->getBuf(60)));// * qSqrt(3));
                inv[index]->acCurrent = client->getBuf(61) * 0x1000000 + client->getBuf(62) * 0x10000 + client->getBuf(63) * 0x100 + client->getBuf(64) == 65535 ? 0 : client->getBuf(61) * 0x1000000 + client->getBuf(62) * 0x10000 + client->getBuf(63) * 0x100 + client->getBuf(64);

                inv[index]->acFrequency = client->getBuf(79) * 0x1000000 + client->getBuf(80) * 0x10000 + client->getBuf(81) * 0x100 + client->getBuf(82) == 65535 ? 0 : client->getBuf(79) * 0x1000000 + client->getBuf(80) * 0x10000 + client->getBuf(81) * 0x100 + client->getBuf(82);

            }

            else if (selectSendMsgType == 3)
            {
                inv[index]->operatingStatus = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
                inv[index]->operatingStatus1 = client->getBuf(13) * 0x1000000 + client->getBuf(14) * 0x10000 + client->getBuf(15) * 0x100 + client->getBuf(16);
            }

            client->TcpDisconnect();
            */
        }
    }

    delete client;


    return check;
}




//1초 마다 확인
void MainWindow::cheslot()
{
    ui->label_3->setText("server  : "+QString::number(serTimer->remainingTime()/1000));//
    ui->label_4->setText("inverter : "+QString::number(invTimer->remainingTime()/1000));//

    if(digitalRead(1)==1)
    {


        digitalWrite(0,(jj%2)); //외부 버튼을 이용한 리셋 기능
        qDebug()<<"jj : "<<(jj%2);

        jj++;

    }


    if(digitalRead(7)==1 && black == false)
    {
        black = true;
        if(toggle==true){
            SendWCDMA();
            //QFuture<void> th5 = QtConcurrent::run(MainWindow::SendWCDMA);
            //send_watcher.setFuture(th5);
        }
        else{
            SendServerHstec();
        }
    }

    else if (digitalRead(7)==0 && black == true)
    {
        black = false;
        if(toggle==true){
            SendWCDMA();
            //QFuture<void> th5 = QtConcurrent::run(MainWindow::SendWCDMA);
            //send_watcher.setFuture(th5);
        }
        else{
            SendServerHstec();
        }
    }

}

//서버 전송
void MainWindow::serslot()
{
    ui->textBrowser->clear();
    ui->textBrowser->append("전송중..");

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

//초기 시작
void MainWindow::strslot()
{
    invTimer->start();
    serTimer->start();
    cheTimer->start();


    strTimer->stop();
    ui->textBrowser->clear();
}












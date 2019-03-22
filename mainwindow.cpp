#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "data.h"
#include "tcpclient.h"

#define INF 1

enum{

    STP25K=0,
    STP50K,
    SC1M,
    STP60K
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
    ui->comboBox->addItem("60K(test)");


    capacity = getFileNum("capacity.txt");
    ui->comboBox->setCurrentIndex(capacity);

    toggle = getFileNum("mode.txt");
    if(toggle==true)
    {
        ui->pushButton_9->setText("WCD");

        if(invCount>10)
            serTimer->setInterval(600000);
        else
            serTimer->setInterval(300000);
    }
    else
    {
        serTimer->setInterval(600000);
        ui->pushButton_9->setText("LAN");
    }


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
    }

    //50K
    else if(capacity==STP50K)
    {
        inv50K();
    }

    //1M
    else if(capacity==SC1M)
    {
        inv1M();
    }

    //60K
    else if(capacity==STP60K)
    {
        inv60K();
    }
}

//메세지를 담는 부분
bool MainWindow::SendMessage(QString ipAddress, int selectSendMsgType, int index)
{

    //TcpClient *client = new TcpClient();
    //bool check=true;
    //bool check = client->TcpConnect(ipAddress,502);


        //25K STP
        if(capacity==STP25K)
        {
            return SendMessage25K(ipAddress,selectSendMsgType,index);
        }

        //50K STP
        else if(capacity==STP50K)
        {
            return SendMessage50K(ipAddress,selectSendMsgType,index);
        }

        //1M SC
        else if(capacity==SC1M)
        {
            return SendMessage1M(ipAddress,selectSendMsgType,index);
        }

        else if(capacity==STP60K)
        {
            return SendMessage60K(ipAddress,selectSendMsgType,index);
        }



    return true;
}

//용량 선택
void MainWindow::on_comboBox_activated(const QString &arg1)
{
    if(arg1 == "25K")
    {
        QMessageBox::information(this,"info",arg1,"OK");
        capacity=0;
    }
    else if (arg1 == "50K")
    {
        QMessageBox::information(this,"info",arg1,"OK");
        capacity=1;
    }
    else if (arg1 == "1M")
    {
        QMessageBox::information(this,"info",arg1,"OK");
        capacity=2;
    }

    else if (arg1 == "60K(test)")
    {
        QMessageBox::information(this,"info",arg1,"OK");
        capacity=3;
    }

    setFileNum("capacity.txt",capacity);
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
            //serTimer->setInterval(300000);

        }
        else{
            SendServerHstec();
            //serTimer->setInterval(600000);
        }
    }

    else if (digitalRead(7)==0 && black == true)
    {
        black = false;
        if(toggle==true){
            SendWCDMA();
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
       if(sms_watcher.isRunning()==false)
       {
            QFuture<void> th2 = QtConcurrent::run(MainWindow::SendWCDMA);
            send_watcher.setFuture(th2);
       }
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












#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "data.h"
#include "tcpclient.h"

extern int selectSendMsgType;
extern int invCount;
extern bool first;
extern int mode;

extern Inverter* inv[20];

extern int errorCount;
extern bool errorFlag;

int list[10] = {0x86, 0x7e, 0x7f, 0x84, 0x83, 0x82, 0x80, 0x85, 0x87, 0x81};

void MainWindow::inv60K(){

    //50K STP
    if(selectSendMsgType == 1)
    {
        for(int i = 0; i<invCount;i++)
        {
            QtConcurrent::run(MainWindow::SendMessage60K,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType == 2)
    {
        for(int i = 0; i<invCount;i++)
        {
            QtConcurrent::run(MainWindow::SendMessage60K,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType == 3)
    {
        for(int i = 0; i<invCount;i++)
        {
            QFuture<bool> future = QtConcurrent::run(MainWindow::SendMessage60K,invIP[i],selectSendMsgType,i);

            if(future.result()==false&&i==0){
                errorCount++;

                if(errorCount>30)
                    errorFlag=true;
            }
            else{
                errorCount=0;
                errorFlag=false;
            }

            if(errorFlag==true){
                inv[i]->acCurrent=0;
                inv[i]->acCurrent2=0;
                inv[i]->acCurrent3=0;
                inv[i]->acFrequency=0;
                inv[i]->acPower=0;
                inv[i]->acVoltage1=0;
                inv[i]->acVoltage2=0;
                inv[i]->acVoltage3=0;
                inv[i]->operatingStatus=0xff;
                inv[i]->operatingStatus1=0xff;
                inv[i]->operatingStatus2=0xff;
                inv[i]->operatingStatus3=0xff;
                inv[i]->dcCurrent=0;
                inv[i]->dcVoltage=0;
                inv[i]->dcPower=0;
            }

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
            if(inv[i]->operatingStatus == 0xffff) state = "정상";
            else if(first==false)
            {
               state = "ERROR("+QString::number(inv[i]->operatingStatus)+")";
               //setFileLog(invIP[i]+" "+state);
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

            if(mode==WCDMA){
                QFuture<void> th5 = QtConcurrent::run(MainWindow::SendWCDMA);
                send_watcher.setFuture(th5);
            }
            else if(mode==LAN){
                SendServerHstec();
            }
            else{
                QFuture<void> th5 = QtConcurrent::run(MainWindow::SendLTE);
                lsend_watcher.setFuture(th5);
            }

            first=false;
        }

    }

}

//60K STP



bool MainWindow::SendMessage60K(QString ipAddress, int selectSendMsgType, int index){

    char T6SEND[3][12] = {  0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x7e, 0x03, 0x9C, 0xFC, 0x00, 0x0F, // AC Input A,V,W Fre (40189)
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x7e, 0x03, 0x9D, 0x11, 0x00, 0x08, // DC Input A,V,W Total (40210)
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x7e, 0x03, 0x9D, 0x1F, 0x00, 0x03};// Operating status (40224)


    TcpClient *client = new TcpClient();
    ipAddress = "192.168.0.3";
    bool check = client->TcpConnect(ipAddress,502);
    QMutex mutex;

    T6SEND[selectSendMsgType-1][1] = (char) index;

    mutex.lock();

    for(int i = 0; i<3 ; i++)
    {
        T6SEND[i][6] = list[index];
    }

    client->onConnectServer();
    client->sendRequst(T6SEND[selectSendMsgType-1],sizeof(T6SEND[selectSendMsgType-1]));

    client->readMessage();

    mutex.unlock();


    if(check==true)
    {
        if (selectSendMsgType == 2)
        {
            int tempTotal = (client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12));
            if(tempTotal!=0xFFFFFFFF)
                inv[index]->totalYeild = tempTotal/1000;

            inv[index]->dailyYeild = 0;//(client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24));

            inv[index]->dcCurrent = client->getBuf(15) * 0x100 + client->getBuf(16);
            inv[index]->dcCurrent *= 10;
            inv[index]->dcVoltage = client->getBuf(19) * 0x100 + client->getBuf(20);
            inv[index]->dcVoltage *= 10;
            inv[index]->dcPower = client->getBuf(23) * 0x100 + client->getBuf(24);
            inv[index]->dcPower *= 10;
        }
        else if (selectSendMsgType == 1)
        {
            inv[index]->acPower = client->getBuf(31) * 0x100 + client->getBuf(32);
            inv[index]->acPower *= 10;
            inv[index]->acVoltage1 = client->getBuf(23) * 0x100 + client->getBuf(24) == 65535 ? 0 : client->getBuf(23) * 0x100 + client->getBuf(24);// * qSqrt(3));
            inv[index]->acVoltage1 *= 10;
            inv[index]->acVoltage2 = client->getBuf(25) * 0x100 + client->getBuf(26) == 65535 ? 0 : client->getBuf(25) * 0x100 + client->getBuf(26);// * qSqrt(3));
            inv[index]->acVoltage2 *= 10;
            inv[index]->acVoltage3 = client->getBuf(27) * 0x100 + client->getBuf(28) == 65535 ? 0 : client->getBuf(27) * 0x100 + client->getBuf(28);// * qSqrt(3));
            inv[index]->acVoltage3 *= 10;

            inv[index]->acCurrent = client->getBuf(9) * 0x100 + client->getBuf(10) == 65535 ? 0 : client->getBuf(9) * 0x100 + client->getBuf(10);
            inv[index]->acCurrent *= 10;
            inv[index]->acCurrent2 = client->getBuf(11) * 0x100 + client->getBuf(12) == 65535 ? 0 : client->getBuf(11) * 0x100 + client->getBuf(12);
            inv[index]->acCurrent2 *= 10;
            inv[index]->acCurrent3 = client->getBuf(13) * 0x100 + client->getBuf(14) == 65535 ? 0 : client->getBuf(13) * 0x100 + client->getBuf(14);
            inv[index]->acCurrent3 *= 10;

            inv[index]->acFrequency = client->getBuf(35) * 0x100 + client->getBuf(36) == 65535 ? 0 : client->getBuf(35) * 0x100 + client->getBuf(36);

        }

        else if (selectSendMsgType == 3)
        {
            inv[index]->operatingStatus = client->getBuf(9) * 0x100 + client->getBuf(10);
            inv[index]->operatingStatus1 = client->getBuf(11) * 0x100 + client->getBuf(12);
            inv[index]->operatingStatus2 = 0;
            inv[index]->operatingStatus3 = 0;
        }
    }
    client->TcpDisconnect();


    delete client;

    return check;
}

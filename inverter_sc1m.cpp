#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "data.h"
#include "tcpclient.h"

extern int selectSendMsgType;
extern int invCount;
extern bool first;
extern bool toggle;

extern Inverter* inv[20];

void MainWindow::inv1M(){

    if(selectSendMsgType == 1)
    {
        for(int i = 0; i<invCount;i++)
        {
            //C1SEND[selectSendMsgType-1][1] = (char) i;
            QtConcurrent::run(MainWindow::SendMessage1M,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType == 2)
    {
        for(int i = 0; i<invCount;i++)
        {
            //C1SEND[selectSendMsgType-1][1] = (char) i;
            QtConcurrent::run(MainWindow::SendMessage1M,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType ==3)
    {
        for(int i = 0; i<invCount;i++)
        {
            //C1SEND[selectSendMsgType-1][1] = (char) i;
            QtConcurrent::run(MainWindow::SendMessage1M,invIP[i],selectSendMsgType,i);

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
    else if(selectSendMsgType == 4)
    {
        for(int i = 0; i<invCount;i++)
        {
            //C1SEND[selectSendMsgType-1][1] = (char) i;
            QtConcurrent::run(MainWindow::SendMessage1M,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType=1;
    }

}

//1M SC
char C1SEND[4][12] = {  0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x77, 0x43, 0x00, 0x0a, // Yield Daily, Total (30531)
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x78, 0x31, 0x00, 0x46, // DC Input A,V,W A (30769)
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x76, 0x21, 0x00, 0x0a,// Operating status (30241)
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x76, 0x2B, 0x00, 0x0a};// Operating status (30251)



bool MainWindow::SendMessage1M(QString ipAddress, int selectSendMsgType, int index){

    TcpClient *client = new TcpClient();
    bool check = client->TcpConnect(ipAddress,502);

    C1SEND[selectSendMsgType-1][1] = (char) index;

    client->onConnectServer();
    client->sendRequst(C1SEND[selectSendMsgType-1],sizeof(C1SEND[selectSendMsgType-1]));


    client->readMessage();

    if(check==true)
    {
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
            inv[index]->acCurrent2 = inv[index]->acCurrent;
            inv[index]->acCurrent3 = inv[index]->acCurrent;

            inv[index]->acFrequency = client->getBuf(79) * 0x1000000 + client->getBuf(80) * 0x10000 + client->getBuf(81) * 0x100 + client->getBuf(82) == 65535 ? 0 : client->getBuf(79) * 0x1000000 + client->getBuf(80) * 0x10000 + client->getBuf(81) * 0x100 + client->getBuf(82);

        }

        else if (selectSendMsgType == 3)
        {
            inv[index]->operatingStatus = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
            inv[index]->operatingStatus1 = client->getBuf(13) * 0x1000000 + client->getBuf(14) * 0x10000 + client->getBuf(15) * 0x100 + client->getBuf(16);
        }

        else if (selectSendMsgType == 4)
        {
            inv[index]->operatingStatus2 = client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12);
        }
    }
    client->TcpDisconnect();


    delete client;

    return check;
}

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

void MainWindow::inv50K(){

    //50K STP
    if(selectSendMsgType == 1)
    {
        for(int i = 0; i<invCount;i++)
        {
            //if(i==7) QtConcurrent::run(MainWindow::SendMessage25K,invIP[i],selectSendMsgType,i);
            //else QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);
            QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType == 2)
    {
        for(int i = 0; i<invCount;i++)
        {
            //if(i==7) QtConcurrent::run(MainWindow::SendMessage25K,invIP[i],selectSendMsgType,i);
            //else QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);
            QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType == 3)
    {
        for(int i = 0; i<invCount;i++)
        {
            //if(i==7) QtConcurrent::run(MainWindow::SendMessage25K,invIP[i],selectSendMsgType,i);
            //else QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);
            QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType++;
    }
    else if(selectSendMsgType ==4)
    {
        for(int i = 0; i<invCount;i++)
        {
            QFuture<bool> future;
            //if(i==7) future = QtConcurrent::run(MainWindow::SendMessage25K,invIP[i],selectSendMsgType,i);
            //else future = QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);
            future = QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);

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
            if(inv[i]->operatingStatus == 0x127) state = "발전";
            else if(inv[i]->operatingStatus == 0xFFFD) state = "정지";
            else if(inv[i]->operatingStatus == 0x5BB) state = "준비";
            else if(inv[i]->operatingStatus == 0xFF) state = "통신에러";
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
        selectSendMsgType++;

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
    else if(selectSendMsgType == 5)
    {
        for(int i = 0; i<invCount;i++)
        {
            //if(i==7) QtConcurrent::run(MainWindow::SendMessage25K,invIP[i],selectSendMsgType,i);
            //else QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);
            QtConcurrent::run(MainWindow::SendMessage50K,invIP[i],selectSendMsgType,i);
        }
        selectSendMsgType=1;
    }
}

//50K STP
char T5SEND[5][12] = {  0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x77, 0x43, 0x00, 0x0a, // Yield Daily, Total (30531)
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x78, 0x31, 0x00, 0x46, // DC Input A,V,W A (30769)
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x78, 0xed, 0x00, 0x12, // DC Input A,V,W B (30957)
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x9c, 0x5d, 0x00, 0x0a, // Operating status (40029)
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x04, 0x79, 0xe9, 0x00, 0x0C};// DC Input A,V,W B (31209)


bool MainWindow::SendMessage50K(QString ipAddress, int selectSendMsgType, int index){

    TcpClient *client = new TcpClient();
    bool check = client->TcpConnect(ipAddress,502);

    T5SEND[selectSendMsgType-1][1] = (char) index;

    client->onConnectServer();
    client->sendRequst(T5SEND[selectSendMsgType-1],sizeof(T5SEND[selectSendMsgType-1]));


    client->readMessage();
    if(check==true)
    {
        if (selectSendMsgType == 1)
        {
            int tempTotal = (client->getBuf(9) * 0x1000000 + client->getBuf(10) * 0x10000 + client->getBuf(11) * 0x100 + client->getBuf(12));
            if(tempTotal!=0xFFFFFFFF)
                inv[index]->totalYeild = tempTotal;

            inv[index]->dailyYeild = 0;//(client->getBuf(21) * 0x1000000 + client->getBuf(22) * 0x10000 + client->getBuf(23) * 0x100 + client->getBuf(24));
            //modi
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
            inv[index]->acCurrent2 = inv[index]->acCurrent;
            inv[index]->acCurrent3 = inv[index]->acCurrent;

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
            inv[index]->operatingStatus1 = 0;
            inv[index]->operatingStatus2 = 0;
            inv[index]->operatingStatus3 = 0;
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
    }
    client->TcpDisconnect();

    delete client;

    return check;

}

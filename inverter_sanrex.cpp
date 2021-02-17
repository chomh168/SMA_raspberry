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

void MainWindow::SANREX(){
    if(selectSendMsgType == 1)
    {
        for(int i = 0; i<invCount;i++)
        {
            QFuture<bool> future = QtConcurrent::run(MainWindow::SendMessageSANREX,invIP[i],selectSendMsgType,i);

            if(future.result()==false&&i==0){
                errorCount++;

                if(errorCount>30)
                    errorFlag=true;
            }
            else{
                errorCount=0;
                errorFlag=false;
            }

            QStandardItem *Item = new QStandardItem(QString::number(inv[i]->totalYeild));
            model->setItem(i,9,Item);
            Item = new QStandardItem(QString::number(inv[i]->dailyYeild));
            model->setItem(i,8,Item);

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
            Item = new QStandardItem(QString::number(inv[i]->acCurrent / 1000));
            model->setItem(i,5,Item);
            Item = new QStandardItem(QString::number(inv[i]->acPower / 1000));
            model->setItem(i,6,Item);
            Item = new QStandardItem(QString::number(inv[i]->acVoltage1 / 100));
            model->setItem(i,4,Item);

            QString state="Null";
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



//25K STP
char SSEND[6] = {  0x4d, 0x4e, 0x20, 0x30, 0x31, 0x0d }; // Operating status1 (30251)


bool MainWindow::SendMessageSANREX(QString ipAddress, int selectSendMsgType, int invno){

    if(wiringPiSetup() == -1){
        return false;
    }

    uart_ch_usb(SSEND,invno);
    
    return check;
}

int HexString_to_int(char byte_4, char byte_3, char byte_2, char byte_1)
{
	int result=0;
	char rxStr[5]={0,};

	rxStr[0] = byte_4;
	rxStr[1] = byte_3;
	rxStr[2] = byte_2;
	rxStr[3] = byte_1;

    result = strtol(rxStr,NULL,16);

	return result;
}

//유아트 전송
QString MainWindow::uart_ch_usb(char *ch, int invno)
{
    char rxbuffer[1024] = {0,};
    int fd;
    int count=0;

    char buf;
    unsigned int checksum = 0; 
    char* command;

    if(invno == 0) sprintf(command, "sudo chmod 777 /dev/ttyUSB0");
    if(invno == 1) sprintf(command, "sudo chmod 777 /dev/ttyUSB1");
    if(invno == 2) sprintf(command, "sudo chmod 777 /dev/ttyUSB2");
    else return false;

    if((fd=serialOpen(command, 115200))<0)
    {
        qDebug()<<"err:not Open";
        return false;
    }

    system(command);
    serialPuts(fd,ch);

    QThread::sleep(1);

    while(serialDataAvail(fd)!='\0')
    {
        system(command);
        buf = serialGetchar(fd);
        rxbuffer[count++]=buf;
        checksum ^= buf;
    }

    if(count==91){
        if(rxbuffer[0]=='M' && rxbuffer[1]=='N'){
            inv[invno]->dcVoltage = HexString_to_int(rxbuffer[73],rxbuffer[74],rxbuffer[75],rxbuffer[76]); // /10
            inv[invno]->dcCurrent = HexString_to_int(rxbuffer[78],rxbuffer[79],rxbuffer[80],rxbuffer[81]); // /10
            inv[invno]->dcPower = inv[invno]->dcVoltage * inv[invno]->dcCurrent; // /100

            inv[invno]->acVoltage1 = HexString_to_int(rxbuffer[58],rxbuffer[59],rxbuffer[60],rxbuffer[61]);
            inv[invno]->acVoltage2 = inv[invno]->acVoltage1;
            inv[invno]->acVoltage3 = inv[invno]->acVoltage1;

            inv[invno]->acCurrent  = HexString_to_int(rxbuffer[3],rxbuffer[4],rxbuffer[5],rxbuffer[6]);
            inv[invno]->acCurrent2 = inv[invno]->acCurrent;
            inv[invno]->acCurrent3 = inv[invno]->acCurrent;

            inv[invno]->acPower = HexString_to_int(rxbuffer[8],rxbuffer[9],rxbuffer[10],rxbuffer[11]);

            inv[invno]->acFrequency = HexString_to_int(rxbuffer[48],rxbuffer[49],rxbuffer[50],rxbuffer[51]);

            inv[invno]->operatingStatus  = HexString_to_int(rxbuffer[83],rxbuffer[84],rxbuffer[85],rxbuffer[86]);
            inv[invno]->operatingStatus1 = inv[invno]->operatingStatus;
            inv[invno]->operatingStatus2 = inv[invno]->operatingStatus;
            inv[invno]->operatingStatus3 = inv[invno]->operatingStatus;

            inv[invno]->totalYeild = HexString_to_int(rxbuffer[18],rxbuffer[19],rxbuffer[20],rxbuffer[21]);
            inv[invno]->dailyYeild = HexString_to_int(rxbuffer[28],rxbuffer[29],rxbuffer[30],rxbuffer[31]);
        }
    }

    serialFlush(fd);
    serialClose(fd);

    return 0;
}

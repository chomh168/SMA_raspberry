#include "mainwindow.h"
#include "data.h"
#include "ui_mainwindow.h"

#define INF 1

extern int NCSQ;

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

    return str;
}

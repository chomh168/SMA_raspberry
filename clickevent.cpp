#include "mainwindow.h"
#include "data.h"
#include "ui_mainwindow.h"

extern int plantNumber;
extern int invCount;

extern int capacity;
extern int mode;

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

    if(mode==WCDMA)
    {
        if(invCount>=7)
            serTimer->setInterval(600000);
        else
            serTimer->setInterval(300000);
    }
    else if(mode==LAN)
    {
        serTimer->setInterval(600000);
    }
    else if(mode==LTE)
    {
        if(invCount>=7)
            serTimer->setInterval(600000);
        else
            serTimer->setInterval(300000);
    }

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

//서버 전송
void MainWindow::on_pushButton_6_clicked()
{
    if(send_watcher.isRunning()==false||lsend_watcher.isRunning()==false)
    {
        serslot();

        if(mode==WCDMA)
        {
            serTimer->setInterval(600000);
        }
        else if(mode==LAN)
        {
            if(invCount>=7)
                serTimer->setInterval(600000);
            else
                serTimer->setInterval(300000);
        }
        else if(mode==LTE)
        {
            if(invCount>=7)
                serTimer->setInterval(600000);
            else
                serTimer->setInterval(300000);
        }
    }
    else
        QMessageBox::information(this,"nofi","전송중","OK");



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
    //mode 현재상태
    if(mode==WCDMA)
    {
        mode++;
        ui->pushButton_9->setText("LAN");
        setFileNum("mode.txt",mode);

        serTimer->setInterval(600000);
    }
    else if(mode==LAN)
    {
        mode++;
        ui->pushButton_9->setText("LTE");
        setFileNum("mode.txt",mode);

        if(invCount>=7)
            serTimer->setInterval(600000);
        else
            serTimer->setInterval(300000);
    }
    else if(mode==LTE)
    {
        mode=0;
        ui->pushButton_9->setText("WCD");
        setFileNum("mode.txt",mode);

        if(invCount>=7)
            serTimer->setInterval(600000);
        else
            serTimer->setInterval(300000);
    }
}



//HW 리셋
void MainWindow::hw_reset()
{
    digitalWrite(0,(0)); //외부 버튼을 이용한 리셋 기능
    QThread::sleep(1);
    digitalWrite(0,(1));
}

//SW 리셋
void MainWindow::sw_reset()
{
    system("reboot");
}

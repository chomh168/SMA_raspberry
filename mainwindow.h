#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include <QTimer>
#include <QtMath>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QThread>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <QFutureWatcher>
#include <QStandardItemModel>
#include <QDateTime>




namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int getFileNum(QString str);
    void setFileNum(QString str,int Num);
    void setFileLog(QString log);

    QTimer *invTimer;
    QTimer *serTimer;
    QTimer *cheTimer;
    QTimer *strTimer;
    QTimer *stpTimer;


    static bool SendMessage(QString server, int sendNum, int index);

    static bool SendMessage25K(QString server, int sendNum, int index);
    static bool SendMessage50K(QString server, int sendNum, int index);
    static bool SendMessage1M(QString server, int sendNum, int index);

    void inv25K();
    void inv50K();
    void inv1M();

    void SendServerHstec();
    void SendMessageHstec(QString server, int port, char *data,int size);


    static void SendWCDMA();
    static QString uart_ch(char *ch,int state);
    static void send_append(char *TCPWRITE);
    static QString req_csq();

    static void SMSReceive();

    QStandardItemModel *model;

    QFutureWatcher<QString> watcher;
    QFutureWatcher<void> send_watcher;
    QFutureWatcher<void> sms_watcher;


    QString csq;



private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_comboBox_activated(const QString &arg1);

public slots:
    void serslot();
    void invslot();
    void cheslot();
    void strslot();
    void csq_ok();
    void send_ok();
    void hw_reset();
    void sw_reset();



private:
    Ui::MainWindow *ui;



public:

    int selectSendMsgType = 1;

    QString invIP[20]  = {
                /*"192.168.0.2",
                "192.168.0.3",
                "192.168.0.4",
                "192.168.0.5",
                "192.168.0.6",
                "192.168.0.7",
                "192.168.0.8",
                "192.168.0.9",
                "192.168.0.10",
                "192.168.0.11",
                "192.168.0.12",
                "192.168.0.13",
                "192.168.0.14",
                "192.168.0.15",
                "192.168.0.16",
                "192.168.0.17",
                "192.168.0.18",
                "192.168.0.19",
                "192.168.0.20",
                "192.168.0.21"*/

                //"172.30.1.5"

                "192.168.0.101",
                "192.168.0.102",
                "192.168.0.103",
                "192.168.0.104",
                "192.168.0.105",
                "192.168.0.106",
                "192.168.0.107",
                "192.168.0.108",
                "192.168.0.109",
                "192.168.0.110",
                "192.168.0.111",
                "192.168.0.112",
                "192.168.0.113",
                "192.168.0.114",
                "192.168.0.115",
                "192.168.0.116",
                "192.168.0.117",
                "192.168.0.118",
                "192.168.0.119",
                "192.168.0.120"
            };

    int jj=0;


    char buff[256];
};




#endif // MAINWINDOW_H

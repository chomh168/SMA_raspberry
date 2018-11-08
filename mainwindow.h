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

    //void SendMessage(QString server, char* data, int size);
    static bool SendMessage(QString server, int sendNum, int index);

    void SendServerHstec();
    void SendMessageHstec(QString server, int port, char *data,int size);

    static void SendWCDMA();
    static QString uart_ch(char *ch,int state);
    static void send_append(char *TCPWRITE);
    static QString req_csq();

    QStandardItemModel *model;

    QFutureWatcher<QString> watcher;
    QFutureWatcher<void> send_watcher;
    QString csq;

    QAction *pSlotTest1;


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

public slots:
    void serslot();
    void invslot();
    void cheslot();
    void strslot();
    void csq_ok();
    void send_ok();

public slots:



private:
    Ui::MainWindow *ui;




private:
    int nport;
    int ninverter;

};




#endif // MAINWINDOW_H

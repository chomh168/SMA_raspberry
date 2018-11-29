#include "mainwindow.h"
#include "tcpclient.h"

//번호 받기
int MainWindow::getFileNum(QString str)
{
    QString filename = str;
    QFile File(QApplication::applicationDirPath() + "/" + filename);

    if(!File.open(QFile::ReadOnly|QFile::Text)) // 읽기 전용, 텍스트로 파일 열기
    {
         if(!File.exists()) // 파일이 존재하지 않으면...
        {
             if(str=="port.txt")
                return 7777;
             else
                 return 1;
        }

    }

    QTextStream OpenFile(&File);
    QString ConfigText;
    while(!OpenFile.atEnd())  // 파일 끝까지 읽어서
    {
         ConfigText=OpenFile.readLine(); // 한라인씩 읽어서 변수에 적용
    }
    File.close(); // 파일닫기

    return ConfigText.toInt();
}

//번호 입력
void MainWindow::setFileNum(QString str, int Num)
{
    QFile *file = new QFile;

    file->setFileName(QApplication::applicationDirPath() + "/" + str); //파일의 위치와 파일명 설정

    if (!file->open(QIODevice::WriteOnly)) //파일을 오픈
    {
        qDebug() << "Error: File Not open";
    }

    QString strNum(QString::number(Num)); //파일에 작성할 테스트
    file->write(strNum.toUtf8()); //파일에 텍스트를 작성
    file->close();
}

//로그 기록
void MainWindow::setFileLog(QString log)
{
    QFile *file = new QFile;
    QString str = "log.txt";

    file->setFileName(QApplication::applicationDirPath() + "/" + str); //파일의 위치와 파일명 설정

    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) //파일을 오픈
    {
        qDebug() << "Error: File Not open";
    }

    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    file->write(time.toUtf8()+" " + log.toUtf8() + "\r\n"); //파일에 텍스트를 작성
    file->close();
}



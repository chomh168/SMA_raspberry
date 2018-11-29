#include "tcpclient.h"
#include "mainwindow.h"


TcpClient::TcpClient(QObject *parent) : QObject (parent)
{
    m_socket = new QTcpSocket;
}

TcpClient::~TcpClient()
{

}



void TcpClient::setIpAddress(QString address)
{
    m_ipAddress = address;

    return;
}

bool TcpClient::TcpConnect(QString server,int port)
{
    this->setIpAddress(server);
    this->connectToServer(port);

    if(!m_socket->waitForConnected(5000)) // 5초 동안 기다림
    {
            qDebug() << "Error: " << m_socket->errorString();
            setFileLog(server+" "+m_socket->errorString());

            return false;
    }

    return true;
}

void TcpClient::connectToServer(int port)
{
    unsigned short recport=(unsigned short)port;
    m_socket->connectToHost(m_ipAddress,recport);

    if(m_socket->state()== QTcpSocket::ConnectedState)
        m_connectState = true;//onConnectServer();

    return;
}

void TcpClient::TcpDisconnect(void)
{
    if(m_connectState==true)
    {
        m_socket->close();
        m_connectState = false;
    }

    return;
}

void TcpClient::onConnectServer(void)
{
    m_connectState = true;

    return;
}

void TcpClient::sendRequst(char buffer[],int size)
{
    if(m_connectState==true)
    {
        m_socket->write(buffer, size);
    }

}

void TcpClient::readMessage(void)
{

    char chr;

    if(!m_socket->bytesAvailable())
    {
        m_socket->waitForReadyRead(3000); // 3초 동안 기다림
        QByteArray baMessage = m_socket->readAll();


        for(int index = 0; index < baMessage.count();index++)
        {
            chr = baMessage.at(index);
            this->buffer[index]=chr;
        }
    }

}


unsigned char TcpClient::getBuf(int index)
{
    return (unsigned)this->buffer[index];
}


//통신 로그 기록
void TcpClient::setFileLog(QString log)
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


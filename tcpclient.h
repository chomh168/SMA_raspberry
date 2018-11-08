#ifndef TCPCLINET_H
#define TCPCLINET_H

#include<QObject>
#include<QTcpSocket>
#include<QHostAddress>
#include<QByteArray>
#include<QDataStream>
#include <QApplication>


class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();
    QTcpSocket *m_socket;
    void setIpAddress(QString address);

    bool TcpConnect(QString server,int port);
    void TcpDisconnect(void);
    void onConnectServer(void);
    void sendRequst(char* buffer,int size);
    void connectToServer(int port);
    void readMessage(void);
    bool getConnect();
    unsigned char getBuf(int index);
    void setFileLog(QString log);



signals:

public slots:

private:
    QString m_ipAddress;
    bool m_connectState;
    char buffer[256];


};



#endif // TCPCLINET_H

#ifndef NET_H
#define NET_H

#include <QWidget>
#include "stl.h"


class Net : public QWidget {
    Q_OBJECT
    public:

      explicit Net(QWidget *parent = 0);         //构造函数
      static QTcpSocket *tcpSocket;                   //静态成员变量用于网络通信
      QTcpSocket* get_tcp_socket();
      void send_message(QJsonObject json);                             //发出数据
      void handle_message(QJsonObject json);                           //处理数据
  
private:
      quint16 blockSize = 0;                          //发送或者接受数据段长度
private: signals:
      send_json(QJsonObject json);

    public slots:
      void read_message();                             //收到数据的信号
};

#endif // NET_H

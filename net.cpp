#include "net.h"

//构造函数
Net::Net(QWidget *parent) : QWidget(parent) {
    connect(Net::tcpSocket, SIGNAL(readyRead()), this,SLOT(read_message()));
}

//读取数据信号
void Net::read_message() {
    QDataStream in(tcpSocket);
    QString message;
    in.setVersion(QDataStream::Qt_5_5);

    //表示没开始读数据块
    if (blockSize == 0) {
        if (tcpSocket->bytesAvailable() < sizeof(quint16)) {
            qDebug() << "return1";
            return;
        }
        in >> blockSize;
        //qDebug() << blockSize;

    }
    //表示没有读完
    if (tcpSocket->bytesAvailable() < blockSize) {
        qDebug() << tcpSocket->bytesAvailable();
        qDebug() << blockSize;
        qDebug() << "return2";
        return;
    }
    in >> message;
    QByteArray string1 = message.toUtf8();
    QJsonDocument parseDocument = QJsonDocument::fromJson(string1);
    QJsonObject obj = parseDocument.object();
    //这里输出收到的json数据
    qDebug()<<obj<<endl;
    handle_message(obj);
    //将BlockSize置为0
    blockSize = 0;
}

//发送Json给服务器
void Net::send_message(QJsonObject json) {

    qDebug() << "发送信息：JSON： ";
    qDebug() << json;                                    //输出Json信息
    QJsonDocument jdoc;
    jdoc.setObject(json);
    QByteArray byte_array = jdoc.toJson(QJsonDocument::Compact); //转化为字节串
    QString json_str(byte_array);                                //转化为字节串

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_5);                         //商定发送和解析协议
    out << (quint16)0;
    out << json_str;
    out.device() ->seek(0);
    out << (quint16)(block. size() - sizeof(quint16));           //插入有用信息的大小

    qDebug()<<Net::tcpSocket->state();

    Net::tcpSocket->write(block);                           //发送信息
    qDebug() << "发送完毕" << endl;
}

void Net::handle_message(QJsonObject json) {
   emit send_json(json);
}

































//初始化
QTcpSocket *Net::tcpSocket = new QTcpSocket;


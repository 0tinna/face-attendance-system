#include "attendancewin.h"
#include "ui_attendancewin.h"
#include <QDateTime>
#include <QSqlRecord>
#include <QThread>
#include <opencv.hpp>
#include <QSqlQuery>
#include <QSqlError>
AttendanceWin::AttendanceWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AttendanceWin)
{
    ui->setupUi(this);
    
    qDebug() << "=== AttendanceServer 启动 ===";
      //qtcpServer当有客户端连会发送newconnection
    connect(&mserver, &QTcpServer::newConnection, this, &AttendanceWin::accept_client);
    
    // 先尝试关闭可能存在的旧连接
    if(mserver.isListening()) {
        mserver.close();
    }
    
    // 等待端口释放
    QThread::msleep(100);
    
    // 尝试在不同端口启动服务器
    bool serverStarted = false;
    for(int port = 9999; port < 10010; ++port) {
        if(mserver.listen(QHostAddress::Any, port)) {
            qDebug() << "=== 服务器成功启动 ===";
            qDebug() << "监听端口:" << port;
            qDebug() << "服务器地址:" << mserver.serverAddress().toString();
            serverStarted = true;
            break;
        }
        qDebug() << "端口" << port << "被占用，尝试下一个端口";
    }
    
    if(!serverStarted) {
        qDebug() << "服务器启动失败:" << mserver.errorString();
    }
    
    bsize = 0;

    //给sql模型绑定表格
    model.setTable("employee");


    //创建一个线程
    QThread *thread = new QThread();
    //把QFaceObject对象移动到thread线程中执行
    fobj.moveToThread(thread);
    //启动线程
    thread->start();
    connect(this,&AttendanceWin::query,&fobj,&QFaceObject::face_query);
    //关联QFaceObject对象里面的send_faceid信号
    connect(&fobj,&QFaceObject::send_faceid,this, &AttendanceWin::recv_faceid);

}

AttendanceWin::~AttendanceWin()
{
    delete ui;
}

//接受客户端连接
void AttendanceWin::accept_client()
{
    //获取与客户端通信的套接字
    msocket = mserver.nextPendingConnection();
    qDebug()<<"新客户端连接，IP："<<msocket->peerAddress().toString()
            <<"端口："<<msocket->peerPort();

    //当客户端有数据到达会发送readyRead信号
    connect(msocket,&QTcpSocket::readyRead,this,&AttendanceWin::read_data);
    
    // 添加断开连接的处理
    connect(msocket,&QTcpSocket::disconnected,[this](){
        qDebug()<<"客户端断开连接";
    });
    
    // 添加错误处理
    connect(msocket,QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            [this](QAbstractSocket::SocketError error){
        qDebug()<<"Socket错误："<<error<<msocket->errorString();
    });
}

//读取客户端发送的数据
void AttendanceWin::read_data()
{
    QDataStream stream(msocket); //把套接字绑定到数据流
    stream.setVersion(QDataStream::Qt_5_14);

    qDebug()<<"收到数据，当前可用字节数："<<msocket->bytesAvailable()<<"期望大小："<<bsize;

    if(bsize == 0){
        if(msocket->bytesAvailable()<(qint64)sizeof(bsize)) {
            qDebug()<<"数据头不完整，等待更多数据";
            return ;
        }
        //采集数据的长度
        stream>>bsize;
        qDebug()<<"读取到数据大小："<<bsize;
    }

    if(msocket->bytesAvailable() < bsize)//说明数据还没有发送完成，返回继续等待
    {
        qDebug()<<"数据不完整，已接收："<<msocket->bytesAvailable()<<"期望："<<bsize;
        return ;
    }
    QByteArray data;
    stream>>data;
    bsize = 0;
    if(data.size() == 0)//没有读取到数据
    {
        qDebug()<<"接收到的数据为空";
        return;
    }

    qDebug()<<"成功接收图片数据，大小："<<data.size()<<"字节";

    //显示图片
    QPixmap mmp;
    mmp.loadFromData(data,"jpg");
    if(mmp.isNull()) {
        qDebug()<<"图片数据解码失败";
        return;
    }
    mmp = mmp.scaled(ui->picLb->size());
    ui->picLb->setPixmap(mmp);
    qDebug()<<"图片显示成功";


    //识别人脸
    cv::Mat faceImage;
    std::vector<uchar> decode;
    decode.resize(data.size());
    memcpy(decode.data(),data.data(),data.size());
    faceImage = cv::imdecode(decode, cv::IMREAD_COLOR);

    if(faceImage.empty()) {
        qDebug()<<"OpenCV 图片解码失败";
        return;
    }
    qDebug()<<"开始人脸识别";
    //int faceid = fobj.face_query(faceImage); //消耗资源较多
    emit query(faceImage);

}

void AttendanceWin::recv_faceid(int64_t faceid)
{
    //qDebug()<<"00000"<<faceid;
    //从数据库中查询faceid对应的个人信息
    //给模型设置过滤器
    qDebug()<<"识别到的人脸id:"<<faceid;
    if(faceid < 0)
    {
        QString sdmsg = QString("{\"employeeID\":\" \",\"name\":\"\",\"department\":\"\",\"time\":\"\"}");
        msocket->write(sdmsg.toUtf8());//把打包好的数据发送给客户端
        return ;
    }
    model.setFilter(QString("faceID=%1").arg(faceid));
    //查询
    model.select();
    //判断是否查询到数据
    if(model.rowCount() == 1)
    {
        //工号，姓名，专业，时间
        //{employeeID:%1,name:%2,department:现代通信工程,time:%3}
        QSqlRecord record = model.record(0);
        QString sdmsg = QString("{\"employeeID\":\"%1\",\"name\":\"%2\",\"department\":\"现代通信工程\",\"time\":\"%3\"}")
                .arg(record.value("employeeID").toString()).arg(record.value("name").toString())
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

        //把数据写入数据库--考勤表
        QString insertSql = QString("insert into attendance(employeeID) values('%1')").arg(record.value("employeeID").toString());
        QSqlQuery query;
        if(!query.exec(insertSql))
        {
            QString sdmsg = QString("{\"employeeID\":\" \",\"name\":\"\",\"department\":\"\",\"time\":\"\"}");
            msocket->write(sdmsg.toUtf8());//把打包好的数据发送给客户端
            qDebug()<<query.lastError().text();
            return ;
        }else
        {
            msocket->write(sdmsg.toUtf8());//把打包好的数据发送给客户端
        }
    }
}

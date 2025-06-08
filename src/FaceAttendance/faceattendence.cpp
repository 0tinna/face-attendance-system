#include "faceattendence.h"
#include "ui_faceattendence.h"
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QBuffer>
#include <QFile>
FaceAttendence::FaceAttendence(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceAttendence)
{    ui->setupUi(this);

#ifdef WIN32
    //导入级联分类器文件
    cascade.load("C:/env/opencv452/etc/haarcascades/haarcascade_frontalface_default.xml");

    // 创建QCamera相关对象
    camera = new QCamera();
    finder = new QCameraViewfinder();
    imagecapture = new QCameraImageCapture(camera);
    
    // 连接信号槽
    connect(imagecapture, &QCameraImageCapture::imageCaptured, this, &FaceAttendence::showcamera);
    
    camera->setViewfinder(finder);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    imagecapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    
    // 启动摄像头
    camera->start();
#else
    //导入级联分类器文件
    cascade.load("./haarcascade_frontalface_default.xml");
    
    // 创建QCamera相关对象 (Linux)
    camera = new QCamera();
    finder = new QCameraViewfinder();
    imagecapture = new QCameraImageCapture(camera);
    
    // 连接信号槽
    connect(imagecapture, &QCameraImageCapture::imageCaptured, this, &FaceAttendence::showcamera);
    
    camera->setViewfinder(finder);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    imagecapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    
    // 启动摄像头
    camera->start();
#endif
    //启动定时器事件
    startTimer(100);

    //QTcpSocket当断开连接的时候disconnected信号，连接成功会发送connected
    connect(&msocket,&QTcpSocket::disconnected,this, &FaceAttendence::start_connect);
    connect(&msocket,&QTcpSocket::connected,this, &FaceAttendence::stop_connect);
    //关联接收数据的槽函数
    connect(&msocket, &QTcpSocket::readyRead,this, &FaceAttendence::recv_data);

    //定时器连接服务器
    connect(&mtimer, &QTimer::timeout,this,&FaceAttendence::timer_connect);
    //启动定时器
    mtimer.start(5000);//每5s钟连接一次，直到连接成功就不在连接

    flag =0;

    ui->widgetLb->hide();
}

FaceAttendence::~FaceAttendence()
{
    delete ui;
}

void FaceAttendence::timerEvent(QTimerEvent *e)
{
    // 触发图像捕获
    if (imagecapture && camera && camera->state() == QCamera::ActiveState) {
        imagecapture->capture();
    }
    
    // 检查是否有捕获到的图像
    if (img.isNull()) {
        return; // 没有图像数据，直接返回
    }
    
    // 添加额外的安全检查
    if (img.width() <= 0 || img.height() <= 0) {
//        qDebug() << "Invalid image dimensions:" << img.size();
        return;
    }
    
    // 将QImage转换为OpenCV Mat
    Mat srcImage;
    try {
        srcImage = QImageToCvMat(img);
    } catch (const std::exception& e) {
        qDebug() << "Exception in QImageToCvMat:" << e.what();
        return;
    }
    
    // 判断图像是否转换成功
    if (srcImage.empty()) {
        qDebug() << "读取摄像头图像失败：srcImage为空";
        return;
    }

    // 检查图片是否有效
//        if (img.isNull())
//        {
//            qDebug() << "图片无效";
//            return;
//        }

    //把图片大小设与显示窗口一样大
    cv::resize(srcImage,srcImage,Size(480,480));
//    cv::resize(img,img,Size(480,480));

    if(srcImage.data == nullptr) return;
    //把opencv里面的Mat格式数据（BGR）转Qt里面的QImage(RGB)
    cvtColor(srcImage,srcImage, COLOR_BGR2RGB);
    QImage image(srcImage.data,srcImage.cols, srcImage.rows,srcImage.step1(),QImage::Format_RGB888);
    QPixmap mmp = QPixmap::fromImage(image);
    ui->videoLb->setPixmap(mmp);


    Mat grayImage;
    //转灰度图
    cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);
    //检测人脸数据
    std::vector<Rect> faceRects;
    cascade.detectMultiScale(grayImage,faceRects,1.1,3,0,cv::Size(150,150));//检测人脸
    if(faceRects.size()>0 && flag>=0)
    {

        Rect rect = faceRects.at(0);//第一个人脸的矩形框
        //rectangle(srcImage,rect,Scalar(0,0,255));
        //移动人脸框（图片--QLabel）
        ui->headpicLb->move(rect.x,rect.y);

        if(flag > 2){
            //把Mat数据转化为QbyteArray， --》编码成jpg格式
            //std::vector<uchar> buf;
            //cv::imencode(".jpg",srcImage,buf);
            //QByteArray byte((const char*)buf.data(),buf.size());

            //QImage转换为QByteArray
            QByteArray byte;
            QBuffer buffer(&byte);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "jpg");
            buffer.close();

            //准备发送
            quint64 backsize = byte.size();
            QByteArray sendData;
            QDataStream stream(&sendData,QIODevice::WriteOnly);
#ifdef WIN32
            stream.setVersion(QDataStream::Qt_5_14);
#else
            stream.setVersion(QDataStream::Qt_5_7);
#endif
            stream<<backsize<<byte;
            //发送
            msocket.write(sendData);
            flag = -2;
            faceMat = srcImage(rect);
            //保存
            if (!imwrite("./face.jpg", faceMat)) {
                qDebug() << "保存人脸图片失败！";
            }
        }
        flag++;
    }
    if(faceRects.size() == 0)
    {
        //把人脸框移动到中心位置
        ui->headpicLb->move(100,60);
        ui->widgetLb->hide();
        flag=0;
    }
}

void FaceAttendence::recv_data()
{
    //{employeeID:%1,name:%2,department:软件,time:%3}
    QByteArray array = msocket.readAll();
    qDebug()<<array;
    //Json解析
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(array,&err);
    if(err.error != QJsonParseError::NoError)
    {
        qDebug()<<"json数据错误";
        return;
    }

    QJsonObject obj = doc.object();
    QString employeeID = obj.value("employeeID").toString();
    QString name = obj.value("name").toString();
    QString department = obj.value("department").toString();
    QString timestr = obj.value("time").toString();

    ui->numberEdit->setText(employeeID);
    ui->nameEdit->setText(name);
    ui->departmentEdit->setText(department);
    ui->timeEdit->setText(timestr);

//    ui->numberEdit->setText("1");
//    ui->nameEdit->setText("李灏轩");
//    ui->departmentEdit->setText("现代通信工程");
//    ui->timeEdit->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    //通过样式来显示图片
    ui->headLb->setStyleSheet("border-radius:75px;border-image: url(./face.jpg);");
    ui->widgetLb->show();
}

void FaceAttendence::timer_connect()
{
    //连接服务器
    msocket.connectToHost("192.168.1.4",9999);
    qDebug()<<"正在连接服务器";
}

void FaceAttendence::stop_connect()
{
    mtimer.stop();
    qDebug()<<"成功连接服务器";
}

void FaceAttendence::start_connect()
{
    mtimer.start(5000);//启动定时器
    qDebug()<<"断开连接";
}

Mat FaceAttendence::QImageToCvMat(const QImage &image) {
    cv::Mat mat;
    
    // 如果图像为空，返回空的Mat
    if (image.isNull()) {
        qDebug() << "QImage is null!";
        return mat;
    }
    
    // 先打印图像格式和尺寸进行调试
//    qDebug() << "QImage format:" << image.format() << "size:" << image.size();
    
    // 安全的转换方法：统一转换为RGB888格式
    QImage rgbImage;
    
    switch (image.format()) {
    case QImage::Format_RGB888:
        // 已经是RGB888格式，直接使用
        rgbImage = image;
        break;
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_RGB32:
    case QImage::Format_RGBX8888:  // 专门处理这个格式
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
        // 统一转换为RGB888
//        qDebug() << "Converting format" << image.format() << "to RGB888";
        rgbImage = image.convertToFormat(QImage::Format_RGB888);
        break;
    case QImage::Format_Grayscale8:
        // 灰度图直接处理
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, 
                     const_cast<uchar*>(image.bits()), image.bytesPerLine());
        return mat.clone();
    default:
        // 其他格式都转换为RGB888
        qDebug() << "Converting unsupported format" << image.format() << "to RGB888";
        rgbImage = image.convertToFormat(QImage::Format_RGB888);
        break;
    }
    
    // 检查转换后的图像是否有效
    if (rgbImage.isNull()) {
        qDebug() << "Failed to convert QImage to RGB888 format!";
        return mat;
    }
    
    // 验证图像数据
    if (rgbImage.bits() == nullptr || rgbImage.width() <= 0 || rgbImage.height() <= 0) {
        qDebug() << "Invalid image data after conversion!";
        return mat;
    }
    
    try {
        // 创建Mat时使用安全的方式
        mat = cv::Mat(rgbImage.height(), rgbImage.width(), CV_8UC3);
        
        // 手动复制数据，避免内存对齐问题
        for (int y = 0; y < rgbImage.height(); ++y) {
            const uchar* srcLine = rgbImage.scanLine(y);
            uchar* dstLine = mat.ptr<uchar>(y);
            
            for (int x = 0; x < rgbImage.width(); ++x) {
                // RGB -> BGR
                dstLine[x * 3 + 0] = srcLine[x * 3 + 2]; // B
                dstLine[x * 3 + 1] = srcLine[x * 3 + 1]; // G
                dstLine[x * 3 + 2] = srcLine[x * 3 + 0]; // R
            }
        }
//        qDebug() << "Successfully converted QImage to Mat, size:";
    } catch (const std::exception& e) {
        qDebug() << "Exception during image conversion:" << e.what();
        mat = cv::Mat(); // 返回空Mat
    }
    
    return mat;
}



void FaceAttendence::showcamera(int id ,QImage img)
{
    Q_UNUSED(id)
    this->img=img;
//    ui->label->setPixmap(QPixmap::fromImage(img));
}

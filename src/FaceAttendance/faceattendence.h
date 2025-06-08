#ifndef FACEATTENDENCE_H
#define FACEATTENDENCE_H

#include <QMainWindow>
#include <opencv.hpp>
#include <QTcpSocket>
#include <QTimer>
#include <QWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QVBoxLayout>
#include <QHBoxLayout>


#include <QWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QUrlQuery>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QNetworkReply>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QJsonArray>


using namespace cv;
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class FaceAttendence; }
QT_END_NAMESPACE

class FaceAttendence : public QMainWindow
{
    Q_OBJECT
public:
    FaceAttendence(QWidget *parent = nullptr);
    ~FaceAttendence();
    //定时器事件
    void timerEvent(QTimerEvent *e);
    Mat QImageToCvMat(const QImage &image);
public slots:
    void showcamera(int id, QImage preview);

protected slots:
    void recv_data();
private slots:
    void timer_connect();
    void stop_connect();
    void start_connect();


private:
    Ui::FaceAttendence *ui;
    QCamera *camera;
    QCameraViewfinder *finder;
    QCameraImageCapture *imagecapture;
    QTimer  *captureTimer;
    Mat srcImage;

    //摄像头
    VideoCapture cap;
    //haar--级联分类器
    cv::CascadeClassifier cascade;

    //创建网络套接字，定时器
    QTcpSocket msocket;
    QTimer mtimer;

    //标志是否是同一个人脸进入到识别区域
    int flag;

    //保存人类的数据
    cv::Mat faceMat;

    QImage img;






};
#endif // FACEATTENDENCE_H

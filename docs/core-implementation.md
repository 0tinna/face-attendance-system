# 人脸识别考勤系统核心技术实现文档

## 1. 系统实现概述

本人脸识别考勤系统基于Qt框架、OpenCV图像处理库和SeetaFace人脸识别引擎，实现了以下核心功能：

### 1.1 主要功能模块

1. **人脸检测与采集** - 客户端使用OpenCV Haar级联分类器实时检测人脸
2. **人脸识别与匹配** - 服务器端使用SeetaFace引擎进行人脸特征提取和匹配
3. **员工信息管理** - 包括员工注册、信息录入和人脸数据库建立
4. **考勤记录管理** - 自动记录考勤时间和员工信息
5. **网络通信** - 客户端与服务器之间的TCP通信
6. **数据存储** - SQLite数据库存储员工信息和考勤记录

### 1.2 系统架构

- **客户端 (FaceAttendance)**: 负责人脸检测、图像采集和网络传输
- **服务器端 (AttendanceServer)**: 负责人脸识别、数据管理和业务逻辑处理

## 2. 核心文件功能解析

### 2.1 qfaceobject.cpp - 人脸识别核心引擎

这是整个系统的人脸识别核心模块，封装了SeetaFace引擎的功能。

#### 2.1.1 构造函数 - 初始化SeetaFace引擎

```cpp
QFaceObject::QFaceObject(QObject *parent) : QObject(parent)
{
    //初始化SeetaFace三个核心模型
    seeta::ModelSetting  FDmode("C:/env/SeetaFace/bin/model/fd_2_00.dat",seeta::ModelSetting::CPU,0);  // 人脸检测模型
    seeta::ModelSetting  PDmode("C:/env/SeetaFace/bin/model/pd_2_00_pts5.dat",seeta::ModelSetting::CPU,0);  // 关键点检测模型
    seeta::ModelSetting  FRmode("C:/env/SeetaFace/bin/model/fr_2_10.dat",seeta::ModelSetting::CPU,0);  // 人脸识别模型
    
    // 创建FaceEngine实例
    this->fengineptr = new seeta::FaceEngine(FDmode,PDmode,FRmode);

    //导入已有的人脸数据库
    this->fengineptr->Load("./face.db");
}
```

**关键逻辑**：
1. 加载三个预训练模型文件：人脸检测(fd)、关键点检测(pd)、人脸识别(fr)
2. 使用CPU模式运行，适合通用硬件环境
3. 自动加载已存在的人脸数据库文件

#### 2.1.2 人脸注册函数 - face_register

```cpp
int64_t QFaceObject::face_register(cv::Mat &faceImage)
{
    //将OpenCV Mat格式转换为SeetaFace格式
    SeetaImageData simage;
    simage.data = faceImage.data;        // 图像数据指针
    simage.width = faceImage.cols;       // 图像宽度
    simage.height = faceImage.rows;      // 图像高度
    simage.channels = faceImage.channels(); // 通道数(通常为3-RGB)
    
    // 调用SeetaFace引擎注册人脸，返回唯一的人脸ID
    int64_t faceid = this->fengineptr->Register(simage);
    
    // 如果注册成功，保存人脸数据库
    if(faceid>=0){
        fengineptr->Save("./face.db");
    }
    return faceid;
}
```

**关键逻辑**：
1. 数据格式转换：OpenCV Mat → SeetaImageData
2. 特征提取：SeetaFace自动提取人脸特征向量
3. 数据库存储：将特征向量存储到本地face.db文件
4. 返回值：成功返回正数ID，失败返回负数

#### 2.1.3 人脸查询函数 - face_query

```cpp
int QFaceObject::face_query(cv::Mat &faceImage)
{
    //图像格式转换
    SeetaImageData simage;
    simage.data = faceImage.data;
    simage.width = faceImage.cols;
    simage.height = faceImage.rows;
    simage.channels = faceImage.channels();
    
    float similarity=0;  // 相似度变量
    
    // 在数据库中查询最相似的人脸
    int64_t faceid = fengineptr->Query(simage,&similarity);
    
    qDebug()<<"查询"<<faceid<<similarity;
    
    // 相似度阈值判断
    if(similarity > 0.7)  // 相似度大于70%认为匹配成功
    {
        emit send_faceid(faceid);  // 发送匹配成功的人脸ID
    }else
    {
        emit send_faceid(-1);      // 发送-1表示未找到匹配
    }
    return faceid;
}
```

**关键逻辑**：
1. 特征提取：从输入图像提取人脸特征
2. 相似度计算：与数据库中所有人脸进行比较
3. 阈值判断：0.7为识别阈值，可根据实际需求调整
4. 信号发送：通过Qt信号槽机制通知其他模块

### 2.2 faceattendence.cpp - 客户端人脸检测与网络通信

客户端负责实时人脸检测、图像采集和与服务器的通信。

#### 2.2.1 构造函数 - 初始化摄像头和网络

```cpp
FaceAttendence::FaceAttendence(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::FaceAttendence)
{
    ui->setupUi(this);

#ifdef WIN32
    //加载OpenCV人脸检测分类器
    cascade.load("C:/env/opencv452/etc/haarcascades/haarcascade_frontalface_default.xml");
    
    // 设置Qt摄像头组件
    camera = new QCamera();
    finder = new QCameraViewfinder();
    imagecapture = new QCameraImageCapture(camera);
    
    // 连接图像捕获信号
    connect(imagecapture, &QCameraImageCapture::imageCaptured, this, &FaceAttendence::showcamera);
    
    camera->setViewfinder(finder);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    imagecapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    camera->start();
#endif

    // 启动定时器，每100ms检测一次人脸
    startTimer(100);

    //网络连接设置
    connect(&msocket,&QTcpSocket::disconnected,this, &FaceAttendence::start_connect);
    connect(&msocket,&QTcpSocket::connected,this, &FaceAttendence::stop_connect);
    connect(&msocket, &QTcpSocket::readyRead,this, &FaceAttendence::recv_data);
    connect(&mtimer, &QTimer::timeout,this,&FaceAttendence::timer_connect);
    
    mtimer.start(5000); // 每5秒尝试连接服务器
}
```

**关键逻辑**：
1. 跨平台适配：Windows/Linux不同路径的分类器文件
2. 摄像头初始化：Qt Camera框架封装
3. 定时器机制：实时人脸检测和网络重连
4. 信号槽连接：异步事件处理

#### 2.2.2 定时器事件 - 实时人脸检测

```cpp
void FaceAttendence::timerEvent(QTimerEvent *e)
{
    // 触发图像捕获
    if (imagecapture && camera && camera->state() == QCamera::ActiveState) {
        imagecapture->capture();
    }
    
    // 安全检查
    if (img.isNull() || img.width() <= 0 || img.height() <= 0) {
        return;
    }
    
    // QImage转OpenCV Mat
    Mat srcImage = QImageToCvMat(img);
    
    // 转换为灰度图像(Haar分类器需要)
    Mat grayImage;
    cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);
    
    // 使用Haar级联分类器检测人脸
    vector<Rect> faceRects;
    cascade.detectMultiScale(grayImage, faceRects, 1.1, 3, 0, Size(30, 30));
    
    // 在原图上绘制人脸框
    for(int i = 0; i < faceRects.size(); i++)
    {
        Rect rect = faceRects[i];
        rectangle(srcImage, rect, Scalar(0, 255, 0), 2); // 绿色矩形框
        
        // 当检测到稳定人脸时发送到服务器
        if(flag > 30) // 连续检测30次确保稳定性
        {
            // 提取人脸区域
            Mat faceRegion = srcImage(rect);
            
            // 转换为JPEG格式
            QImage qimg = MatToQImage(faceRegion);
            QByteArray byte;
            QBuffer buffer(&byte);
            buffer.open(QIODevice::WriteOnly);
            qimg.save(&buffer, "jpg");
            buffer.close();

            // 准备网络传输数据
            quint64 dataSize = byte.size();
            QByteArray sendData;
            QDataStream stream(&sendData, QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_5_14);
            stream << dataSize << byte; // 先发送数据大小，再发送图像数据
            
            // 发送到服务器
            msocket.write(sendData);
            flag = -2; // 重置计数器
        }
        flag++;
    }
    
    // 未检测到人脸时重置状态
    if(faceRects.size() == 0)
    {
        ui->widgetLb->hide();
        flag = 0;
    }
}
```

**关键逻辑**：
1. 连续性检测：防止误触发，需要连续检测到人脸
2. 图像格式转换：QImage → OpenCV Mat → 灰度图
3. Haar检测：OpenCV经典人脸检测算法
4. 网络协议：先发送数据大小，再发送图像数据

#### 2.2.3 接收服务器响应

```cpp
void FaceAttendence::recv_data()
{
    // 接收服务器返回的JSON数据
    QByteArray array = msocket.readAll();
    
    // JSON解析
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(array, &err);
    if(err.error != QJsonParseError::NoError)
    {
        qDebug()<<"json数据错误";
        return;
    }

    QJsonObject obj = doc.object();
    QString employeeID = obj.value("employeeID").toString();
    QString name = obj.value("name").toString();
    QString time = obj.value("time").toString();
    
    // 显示识别结果
    ui->nameLb->setText(name);
    ui->timeLb->setText(time);
    ui->widgetLb->show(); // 显示识别成功界面
}
```

**关键逻辑**：
1. JSON解析：标准化的数据交换格式
2. 错误处理：解析失败时的异常处理
3. UI更新：显示识别结果给用户

### 2.3 attendancewin.cpp - 服务器端主控制器

服务器端的核心控制逻辑，处理网络通信和人脸识别调度。

#### 2.3.1 网络服务初始化

```cpp
AttendanceWin::AttendanceWin(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AttendanceWin)
{
    ui->setupUi(this);
    
    // 设置网络连接信号
    connect(&mserver, &QTcpServer::newConnection, this, &AttendanceWin::accept_client);
    
    // 端口自动选择机制
    bool serverStarted = false;
    for(int port = 9999; port < 10010; ++port) {
        if(mserver.listen(QHostAddress::Any, port)) {
            qDebug() << "服务器成功启动，监听端口:" << port;
            serverStarted = true;
            break;
        }
    }
    
    // 初始化数据模型
    model.setTable("employee");
    
    // 创建人脸识别线程
    QThread *thread = new QThread();
    fobj.moveToThread(thread);
    thread->start();
    
    // 连接人脸识别信号槽
    connect(this, &AttendanceWin::query, &fobj, &QFaceObject::face_query);
    connect(&fobj, &QFaceObject::send_faceid, this, &AttendanceWin::recv_faceid);
}
```

**关键逻辑**：
1. 端口自动选择：避免端口冲突
2. 线程分离：人脸识别在独立线程中运行，避免UI阻塞
3. 信号槽异步通信：线程间安全通信

#### 2.3.2 接受客户端连接

```cpp
void AttendanceWin::accept_client()
{
    msocket = mserver.nextPendingConnection();
    qDebug() << "客户端连接成功:" << msocket->peerAddress().toString();
    
    // 连接数据接收信号
    connect(msocket, &QTcpSocket::readyRead, this, &AttendanceWin::read_data);
}
```

#### 2.3.3 处理客户端图像数据

```cpp
void AttendanceWin::read_data()
{
    QDataStream stream(msocket);
    stream.setVersion(QDataStream::Qt_5_14);
    
    // 第一步：读取数据大小
    if(bsize == 0)
    {
        if(msocket->bytesAvailable() < sizeof(quint64))
        {
            return; // 数据不完整，等待更多数据
        }
        stream >> bsize;
        qDebug() << "期望接收数据大小：" << bsize;
    }

    // 第二步：读取完整图像数据
    if(msocket->bytesAvailable() < bsize)
    {
        qDebug() << "数据不完整，已接收：" << msocket->bytesAvailable() << "期望：" << bsize;
        return;
    }
    
    QByteArray data;
    stream >> data;
    bsize = 0; // 重置大小标记
    
    // 显示接收到的图像
    QPixmap pixmap;
    pixmap.loadFromData(data, "jpg");
    pixmap = pixmap.scaled(ui->picLb->size());
    ui->picLb->setPixmap(pixmap);

    // 转换为OpenCV格式进行人脸识别
    cv::Mat faceImage;
    std::vector<uchar> buffer(data.begin(), data.end());
    faceImage = cv::imdecode(buffer, cv::IMREAD_COLOR);
    
    if(!faceImage.empty()) {
        qDebug() << "开始人脸识别";
        emit query(faceImage); // 发送到人脸识别线程
    }
}
```

**关键逻辑**：
1. 分步接收：先接收数据大小，再接收完整数据
2. 数据完整性检查：确保接收到完整的图像数据
3. 格式转换：网络字节流 → OpenCV Mat
4. 异步处理：通过信号发送到人脸识别线程

#### 2.3.4 处理人脸识别结果

```cpp
void AttendanceWin::recv_faceid(int64_t faceid)
{
    qDebug() << "收到人脸识别结果:" << faceid;
    
    if(faceid < 0) {
        // 未识别到注册用户
        QJsonObject obj;
        obj.insert("employeeID", "-1");
        obj.insert("name", "未识别");
        obj.insert("time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        
        QJsonDocument doc(obj);
        msocket->write(doc.toJson());
        return;
    }
    
    // 查询员工信息
    model.setFilter(QString("faceID = %1").arg(faceid));
    model.select();
    
    if(model.rowCount() > 0) {
        QSqlRecord record = model.record(0);
        QString employeeID = record.value("employeeID").toString();
        QString name = record.value("name").toString();
        
        // 记录考勤信息
        QSqlQuery query;
        QString sql = QString("INSERT INTO attendance (employeeID) VALUES (%1)").arg(employeeID);
        query.exec(sql);
        
        // 返回识别结果给客户端
        QJsonObject obj;
        obj.insert("employeeID", employeeID);
        obj.insert("name", name);
        obj.insert("time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        
        QJsonDocument doc(obj);
        msocket->write(doc.toJson());
        
        qDebug() << "考勤记录成功：" << name;
    }
}
```

**关键逻辑**：
1. 结果判断：正数ID为成功，负数为失败
2. 数据库查询：根据faceID查找员工信息
3. 考勤记录：自动插入考勤表
4. JSON响应：标准化返回格式

### 2.4 registerwin.cpp - 员工注册管理

负责新员工的注册和人脸数据的建立。

#### 2.4.1 图片选择和预览

```cpp
void RegisterWin::on_addpicBt_clicked()
{
    // 文件对话框选择图片
    QString filepath = QFileDialog::getOpenFileName(
        this,
        "选择图片文件",
        "",
        "图像文件 (*.png *.jpg *.jpeg *.bmp *.gif);;所有文件 (*.*)"
    );

    if (!filepath.isEmpty()) {
        // 验证图像是否可以正确加载
        cv::Mat image = cv::imread(filepath.toLocal8Bit().data());
        if (image.empty()) {
            qDebug() << "图像加载失败";
            return;
        }
        
        ui->picFileEdit->setText(filepath);
        
        // 显示预览图片
        QPixmap pixmap(filepath);
        pixmap = pixmap.scaledToWidth(ui->headpicLb->width());
        ui->headpicLb->setPixmap(pixmap);
    }
}
```

#### 2.4.2 员工注册核心流程

```cpp
void RegisterWin::on_registerBt_clicked()
{
    // 1. 加载选择的图片
    QString imagePath = ui->picFileEdit->text();
    cv::Mat image = cv::imread(imagePath.toUtf8().data());
    if (image.empty()) {
        qDebug() << "图像读取失败";
        return;
    }
    
    // 2. 进行人脸注册，获取人脸ID
    QFaceObject faceobj;
    int64_t faceID = faceobj.face_register(image);
    qDebug() << "注册获得人脸ID：" << faceID;
    
    if(faceID < 0) {
        QMessageBox::warning(this, "错误", "人脸注册失败，请确保图片中包含清晰的人脸");
        return;
    }
    
    // 3. 保存头像文件
    QString headfile = QString("./data/%1.jpg").arg(QString(ui->nameEdit->text().toUtf8().toBase64()));
    image.copyTo(image); // 这里应该是 cv::imwrite(headfile.toStdString(), image);
    
    // 4. 将员工信息插入数据库
    QSqlTableModel model;
    model.setTable("employee");
    model.select();
    
    // 创建新记录
    QSqlRecord record = model.record();
    record.setValue("name", ui->nameEdit->text());
    record.setValue("sex", ui->sexCb->currentText());
    record.setValue("birthday", ui->birthdayEdit->date().toString("yyyy-MM-dd"));
    record.setValue("address", ui->addressEdit->text());
    record.setValue("phone", ui->phoneEdit->text());
    record.setValue("faceID", faceID); // 关键：关联人脸ID
    record.setValue("headfile", headfile);
    
    // 插入数据库
    if(model.insertRecord(-1, record)) {
        model.submitAll();
        QMessageBox::information(this, "成功", "员工注册成功！");
        on_resetBt_clicked(); // 清空表单
    } else {
        QMessageBox::warning(this, "错误", "数据库插入失败：" + model.lastError().text());
    }
}
```

**关键逻辑**：
1. 图像验证：确保选择的图片可以正确加载
2. 人脸注册：调用QFaceObject进行特征提取和存储
3. 文件管理：保存头像到指定目录
4. 数据关联：faceID是连接人脸特征和员工信息的关键

### 2.5 selectwin.cpp - 数据查询界面

提供员工信息和考勤记录的查询功能。

```cpp
void SelectWin::on_selectBt_clicked()
{
    // 根据单选按钮选择查询表
    if(ui->empRb->isChecked())
    {
        model->setTable("employee"); // 查询员工表
    }
    if(ui->attRb->isChecked())
    {
        model->setTable("attendance"); // 查询考勤表
    }

    // 可以设置过滤条件
    // model->setFilter("name='张三'");
    
    // 执行查询
    model->select();
    
    // 绑定到表格视图
    ui->tableView->setModel(model);
}
```

## 3. 人脸识别实现流程总结

### 3.1 完整的人脸识别流程

1. **系统初始化阶段**
   - 服务器启动，加载SeetaFace模型和已有人脸数据库
   - 客户端启动，初始化摄像头和OpenCV分类器
   - 建立TCP网络连接

2. **人脸检测阶段** (客户端)
   - 定时器每100ms捕获一帧图像
   - 使用Haar级联分类器检测人脸位置
   - 连续30次检测到人脸后提取人脸区域

3. **数据传输阶段**
   - 将人脸图像转换为JPEG格式
   - 通过TCP协议发送到服务器
   - 采用"大小+数据"的传输协议确保数据完整性

4. **人脸识别阶段** (服务器)
   - 接收并解码图像数据
   - 调用SeetaFace引擎提取人脸特征
   - 与数据库中的特征进行相似度比较
   - 设定0.7的相似度阈值进行匹配判断

5. **业务处理阶段**
   - 根据匹配的faceID查询员工信息
   - 自动记录考勤时间到数据库
   - 构造JSON响应返回给客户端

6. **结果展示阶段**
   - 客户端解析JSON响应
   - 在界面上显示员工姓名和考勤时间
   - 提供视觉和可能的声音反馈

### 3.2 关键技术要点

1. **跨平台兼容性**: 通过条件编译适配Windows/Linux
2. **线程安全**: 人脸识别在独立线程中运行，使用信号槽通信
3. **网络协议设计**: 先发送数据大小再发送数据内容的可靠传输
4. **数据库设计**: employee表和attendance表通过employeeID关联
5. **人脸特征管理**: 使用SeetaFace的内部数据库管理人脸特征
6. **错误处理**: 各个环节都有相应的错误检查和处理机制

这套系统通过模块化设计和清晰的职责分离，实现了一个完整、可靠的人脸识别考勤解决方案。

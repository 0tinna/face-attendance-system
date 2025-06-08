# 系统设计文档

本文档详细描述了基于Qt和SeetaFace的人脸考勤系统的设计架构和实现细节。

## 1. 系统架构

### 1.1 总体架构

该人脸考勤系统采用C/S (Client/Server) 架构，分为服务器端和客户端两部分：

- **服务器端 (AttendanceServer)**: 负责人员信息管理、人脸数据库维护、考勤数据存储与统计
- **客户端 (FaceAttendance)**: 负责人脸采集、人脸识别和考勤打卡
- **嵌入式终端 (RK3568)**: 运行客户端应用程序，实现轻量级人脸检测和网络通信

![系统架构图](file:///C:/D-APPLICATION/Qt_rk3568_opencv_Seetaface/docs/images/system_architecture.png)

### 1.2 模块组成

系统主要由以下模块组成：

1. **人脸处理模块**：基于OpenCV和SeetaFace实现人脸检测、特征提取和人脸识别
2. **数据管理模块**：基于SQLite实现人员信息和考勤数据的存储与管理
3. **嵌入式适配模块**：针对RK3568开发板的硬件适配和优化

### 1.3 嵌入式平台设计

本系统专为瑞芯微RK3568开发板设计了适配方案：

1. **硬件适配**：
   
   - 处理器：利用RK3568的四核Cortex-A55处理器
   - GPU：使用Mali-G52 GPU加速人脸检测算法
   - 摄像头：支持USB摄像头和CSI接口摄像头
   - 显示：支持HDMI、MIPI、eDP等多种显示接口

2. **软件优化**：
   
   - 针对ARM架构的算法优化，启用NEON加速指令
   - 资源占用优化，降低内存和CPU消耗
   - Qt界面适配触摸屏操作

3. **网络通信模块**：基于Qt的TCP Socket实现客户端和服务器的通信

4. **用户界面模块**：基于Qt实现图形用户界面

## 2. 模块设计

### 2.1 人脸处理模块 (QFaceObject类)

该模块封装了SeetaFace引擎的功能，主要实现：

- 人脸检测：在图像中定位人脸位置
- 特征提取：提取人脸特征向量
- 人脸识别：比对人脸特征与数据库中已存储的特征

```cpp
class QFaceObject : public QObject {
    Q_OBJECT
public:
    explicit QFaceObject(QObject *parent = nullptr);
    ~QFaceObject();
public slots:
    int64_t face_register(cv::Mat& faceImage);  // 注册人脸，返回人脸ID
    int  face_query(cv::Mat& faceImage);        // 查询人脸，返回匹配的人脸ID
signals:
    void send_faceid(int64_t faceid);           // 发送识别结果信号
private:
    seeta::FaceEngine *fengineptr;              // SeetaFace引擎指针
};
```

### 2.2 数据管理模块

采用SQLite数据库存储员工信息和考勤记录。主要表结构如下：

1. **employee表**：存储员工基本信息
   
   ```sql
   CREATE TABLE employee (
       employeeID INTEGER PRIMARY KEY AUTOINCREMENT,
       name VARCHAR(256), 
       sex VARCHAR(32),
       birthday TEXT, 
       address TEXT, 
       phone TEXT, 
       faceID INTEGER UNIQUE, 
       headfile TEXT
   );
   ```

2. **attendance表**：存储考勤记录
   
   ```sql
   CREATE TABLE attendance (
       attendaceID INTEGER PRIMARY KEY AUTOINCREMENT, 
       employeeID INTEGER,
       attendaceTime TIMESTAMP NOT NULL DEFAULT(datetime('now','localtime'))
   );
   ```

### 2.3 网络通信模块

基于Qt的QTcpServer和QTcpSocket实现服务器和客户端的通信：

1. **服务器端**：
   
   - 监听指定端口
   - 接收客户端连接
   - 处理客户端发送的人脸数据
   - 返回识别结果

2. **客户端**：
   
   - 连接到服务器
   - 发送采集到的人脸数据
   - 接收并显示识别结果

### 2.4 用户界面模块

#### 2.4.1 服务器端界面

- 主界面：显示在线状态、员工列表、考勤记录
- 员工管理界面：添加、修改、删除员工信息
- 考勤统计界面：按日期、部门等统计考勤数据

#### 2.4.2 客户端界面

- 主界面：摄像头预览、人脸检测框、考勤状态显示
- 设置界面：服务器连接配置

## 3. 关键流程

### 3.1 人员注册流程

```
1. 管理员在服务器端选择"添加员工"
2. 输入员工基本信息
3. 上传人脸照片或通过摄像头采集
4. 人脸处理模块提取人脸特征
5. 将特征向量存入数据库
6. 生成人脸ID，关联员工信息
```

### 3.2 考勤打卡流程

```
1. 员工站在客户端摄像头前
2. 客户端捕获人脸图像并定位人脸
3. 将人脸图像发送至服务器
4. 服务器提取人脸特征并与数据库比对
5. 找到匹配结果后，记录考勤时间
6. 返回识别结果给客户端
7. 客户端显示员工信息和考勤成功提示
```

## 4. 技术实现

### 4.1 人脸检测与识别

使用级联分类器进行人脸检测：

```cpp
// 导入级联分类器文件
cascade.load("haarcascade_frontalface_default.xml");
```

使用SeetaFace进行人脸特征提取和比对：

```cpp
int64_t QFaceObject::face_register(cv::Mat& faceImage) {
    // SeetaFace提取特征并注册
    ...
}

int QFaceObject::face_query(cv::Mat& faceImage) {
    // SeetaFace提取特征并在数据库中查找
    ...
}
```

### 4.2 网络通信实现

服务器端监听连接：

```cpp
connect(&mserver, &QTcpServer::newConnection, this, &AttendanceWin::accept_client);

// 服务器监听
for(int port = 9999; port < 10010; ++port) {
    if(mserver.listen(QHostAddress::Any, port)) {
        // 成功监听
        break;
    }
}
```

客户端连接服务器：

```cpp
socket = new QTcpSocket(this);
socket->connectToHost(serverIP, serverPort);
```

### 4.3 数据存储实现

SQLite数据库操作：

```cpp
// 连接数据库
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName("server.db");
db.open();

// 创建表格
QString createsql = "create table if not exists employee(...)";
QSqlQuery query;
query.exec(createsql);
```

## 5. 系统性能与优化

### 5.1 性能指标

- 人脸识别准确率：>98%
- 识别响应时间：<1秒
- 最大支持人员数：10,000人
- 并发处理能力：支持多客户端同时连接

### 5.2 优化策略

1. **多线程处理**：
   
   - 使用单独线程处理人脸识别，避免阻塞UI线程
   - 使用线程池处理多客户端请求

2. **数据库优化**：
   
   - 为faceID和employeeID建立索引
   - 定期维护数据库

3. **网络通信优化**：
   
   - 使用二进制协议减少传输数据量
   - 图像压缩降低带宽需求

## 6. 安全性设计

1. **数据安全**：
   
   - 敏感信息（如照片）加密存储
   - 数据库访问权限控制

2. **通信安全**：
   
   - 可扩展实现TLS/SSL加密通信

3. **用户认证**：
   
   - 服务器管理员账户权限控制
   - 操作日志记录

## 7. 可扩展性

系统设计考虑了未来的扩展需求：

1. **硬件扩展**：
   
   - 支持多种摄像头设备
   - 可集成门禁系统

2. **功能扩展**：
   
   - 可添加移动客户端
   - 可集成工资系统
   - 可扩展在线/远程考勤功能

## 8. 部署架构

### 8.1 典型部署架构

- 服务器：部署在内部网络服务器或云服务器
- 客户端：部署在考勤终端、前台电脑或其他接入点
- 嵌入式终端：部署在RK3568开发板上的专用考勤终端

### 8.2 嵌入式终端部署架构

![RK3568部署架构](file:///C:/D-APPLICATION/Qt_rk3568_opencv_Seetaface/docs/images/rk3568_deployment.png)

#### 8.2.1 RK3568部署方案

1. **硬件配置**
   
   - RK3568开发板作为考勤终端
   - 外接USB/CSI摄像头
   - LCD触摸屏或HDMI显示器
   - 有线/无线网络连接

2. **软件部署**
   
   - 基于Debian/Buildroot的嵌入式Linux系统
   - FaceAttendance客户端应用
   - 系统启动自动运行考勤程序
   - 远程管理服务

3. **网络拓扑**
   
   - 多个RK3568终端通过局域网连接到中央服务器
   - 可选择边缘计算模式：终端进行本地人脸检测，服务器进行人脸识别
   - 支持离线模式：终端本地缓存识别结果，网络恢复后同步

4. **性能优化策略**
   
   - 使用OpenCV的NEON优化版本
   - 针对ARM架构的人脸检测算法优化
   - 图像预处理加速
   - 低功耗运行模式

### 8.3 系统要求

- 服务器：
  
  - CPU: 双核2GHz以上
  - RAM: 4GB以上
  - 存储: 50GB以上
  - 操作系统: Windows Server或Linux

- PC客户端：
  
  - CPU: 双核1.5GHz以上
  - RAM: 2GB以上
  - 摄像头: 720p分辨率以上
  - 操作系统: Windows 10或Linux

- RK3568嵌入式终端：
  
  - CPU: RK3568 (Quad-core Cortex-A55 1.8GHz)
  - GPU: ARM Mali-G52
  - RAM: 2GB/4GB
  - 存储: 32GB eMMC/SD卡
  - 摄像头: 支持USB/CSI接口
  - 操作系统: 嵌入式Linux (Debian/Buildroot)

## 9. 测试策略

1. **单元测试**：针对各个模块的功能测试
2. **集成测试**：客户端和服务器的协同工作测试
3. **性能测试**：高并发、大数据量下的系统表现
4. **用户体验测试**：UI友好性和操作便捷性测试

## 10. 已知问题与解决方案

1. **强光条件下人脸识别问题**：
   
   - 解决方案：增加光线补偿算法

2. **类似人脸识别错误问题**：
   
   - 解决方案：提高匹配阈值，增加多角度验证

3. **网络延迟问题**：
   
   - 解决方案：优化数据传输，添加本地缓存

## 11. 未来发展规划

1. **移动端应用**：开发手机APP客户端
2. **人脸识别算法升级**：持续跟进最新的识别算法
3. **云平台整合**：支持云部署和多地点数据同步
4. **数据分析功能**：增加考勤数据智能分析功能

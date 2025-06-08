# 安装指南

本文档提供安装和配置人脸考勤系统的步骤，包括RK3568嵌入式平台的特殊说明。

## 系统要求

### 硬件要求
#### 开发/测试环境
- 处理器: 现代多核CPU (推荐Intel i5或更高)
- 内存: 8GB RAM或更多
- 存储: 至少1GB可用空间
- 摄像头: 支持720p或更高分辨率

#### 嵌入式部署环境 (RK3568)
- 开发板: 瑞芯微RK3568
- 处理器: Quad-core Cortex-A55 (1.8GHz)
- 内存: 2GB/4GB RAM
- 存储: 至少32GB eMMC/SD卡
- 摄像头: 支持USB摄像头或CSI接口摄像头

### 软件要求
#### 开发/测试环境
- 操作系统: Windows 10/11 或 Linux(Ubuntu 20.04+)
- Qt 5.14.2+
- OpenCV 4.5.2+
- SeetaFace 引擎

#### RK3568嵌入式环境
- 操作系统: Buildroot或Debian系统
- Qt 5.14.2+ (交叉编译或ARM原生编译)
- OpenCV 4.5.2+ (ARM版本)
- SeetaFace 引擎 (ARM版本)
- 必要的驱动: 摄像头驱动、显示驱动等

## 依赖项安装

### Windows 环境

1. **Qt安装**
   - 从[Qt官网](https://www.qt.io/download)下载Qt在线安装程序
   - 安装Qt 5.14.2和MinGW 64位编译器

2. **OpenCV安装**
   - 从[OpenCV官网](https://opencv.org/releases/)下载OpenCV 4.5.2
   - 解压缩到合适的目录，例如`C:/env/opencv452`
   - 添加OpenCV的bin目录到系统Path环境变量：`C:/env/opencv452/bin`

3. **SeetaFace安装**
   - 从[SeetaFace官网](https://github.com/seetaface)下载适合您系统的预编译二进制文件
   - 解压缩到合适的目录
   - 配置系统环境变量以找到SeetaFace库

### Linux 环境

1. **Qt安装**
   ```bash
   sudo apt-get update
   sudo apt-get install build-essential
   sudo apt-get install qt5-default qtcreator
   ```

2. **OpenCV安装**
   ```bash
   sudo apt-get install libopencv-dev
   ```

3. **SeetaFace安装**
   - 从GitHub克隆SeetaFace代码并按照其README编译和安装
   
### RK3568开发板环境

1. **系统准备**
   - 确保RK3568开发板已刷入适当的系统镜像(如Buildroot或Debian)
   - 更新系统并安装必要的开发工具：
   ```bash
   sudo apt-get update
   sudo apt-get install build-essential git cmake
   ```

2. **Qt安装 (ARM原生编译方式)**
   - 在开发板上安装Qt库和开发工具：
   ```bash
   sudo apt-get install qt5-default qtbase5-dev qtdeclarative5-dev
   ```
   - 或使用交叉编译工具链在主机上编译(推荐)

3. **OpenCV安装**
   - 方法1: 使用预编译的二进制包：
   ```bash
   sudo apt-get install libopencv-dev
   ```
   - 方法2: 下载OpenCV源代码在RK3568上编译：
   ```bash
   git clone https://github.com/opencv/opencv.git
   cd opencv
   mkdir build && cd build
   cmake -D CMAKE_BUILD_TYPE=RELEASE -D WITH_IPP=OFF -D WITH_OPENCL=OFF ..
   make -j4
   sudo make install
   ```

4. **SeetaFace安装**
   - 从SeetaFace官方仓库下载源码
   - 在RK3568上编译或交叉编译
   - 设置相关库路径：
   ```bash
   export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/seeta/lib
   ```

5. **摄像头驱动设置**
   - 确认摄像头型号和接口(USB或CSI)
   - 安装必要的驱动程序：
   ```bash
   # 对于USB摄像头
   sudo apt-get install v4l-utils
   # 测试摄像头
   v4l2-ctl --list-devices
   ```

## 编译项目

### 在开发/测试环境中编译

1. 使用Qt Creator打开项目
   - 打开Qt Creator
   - 选择"文件" > "打开文件或项目"
   - 导航到项目目录并打开`src/AttendanceServer/AttendanceServer.pro`和`src/FaceAttendance/FaceAttendance.pro`

2. 配置项目
   - 配置编译器和Qt版本
   - 设置构建目录

3. 构建项目
   - 点击"构建" > "构建项目"或按下Ctrl+B

### 为RK3568交叉编译

1. **设置交叉编译工具链**
   - 安装RK3568的交叉编译工具链
   ```bash
   # 假设工具链已下载到本地
   export PATH=$PATH:/path/to/rk3568/toolchain/bin
   ```

2. **Qt交叉编译配置**
   - 在Qt Creator中创建RK3568的自定义套件(Kit)
   - 设置ARM工具链和交叉编译的Qt库
   - 或使用命令行方式：
   ```bash
   # 配置qmake
   /path/to/qmake-arm -spec linux-arm-gnueabi-g++ ../FaceAttendance/FaceAttendance.pro
   # 编译
   make -j$(nproc)
   ```

3. **编译参数设置**
   - 修改.pro文件以包含RK3568特定的编译标志
   ```
   # 在.pro文件中添加
   contains(QMAKE_HOST.arch, aarch64) {
       DEFINES += RK3568
       # 其他RK3568特定设置
   }
   ```

## 配置系统

### 通用配置

1. **数据库配置**
   - 服务器端应用程序会在首次运行时自动创建SQLite数据库
   - 默认数据库文件名为`server.db`

2. **服务器配置**
   - 服务器默认监听9999端口，如被占用会自动尝试9999-10010端口范围内的其他端口
   - 请确保防火墙允许这些端口的TCP通信

3. **客户端配置**
   - 客户端需要配置服务器的IP地址和端口
   - 默认连接本地服务器(localhost)

### RK3568特定配置

1. **硬件资源访问权限**
   ```bash
   # 确保当前用户可以访问摄像头设备
   sudo usermod -a -G video $USER
   # 重启设备或重新登录使更改生效
   ```

2. **性能优化配置**
   - 调整CPU频率以获得更好的性能：
   ```bash
   # 检查可用的CPU频率
   cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies
   # 设置性能模式
   echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
   ```

3. **显示设置**
   - 如果使用HDMI显示，配置最佳分辨率：
   ```bash
   # 查看可用分辨率
   xrandr
   # 设置分辨率
   xrandr --output HDMI-1 --mode 1920x1080
   ```

4. **自启动配置**
   - 创建自启动脚本以便系统启动时自动运行考勤程序：
   ```bash
   # 创建启动脚本
   echo '#!/bin/bash
   cd /path/to/FaceAttendance
   ./FaceAttendance &' > ~/start_attendance.sh
   chmod +x ~/start_attendance.sh
   
   # 添加到启动项
   mkdir -p ~/.config/autostart
   echo '[Desktop Entry]
   Type=Application
   Exec=/home/user/start_attendance.sh
   Hidden=false
   X-GNOME-Autostart-enabled=true
   Name=Face Attendance
   Comment=Start Face Attendance System' > ~/.config/autostart/face-attendance.desktop
   ```

## 常见问题

### 通用问题

1. **编译错误**
   - 检查Qt、OpenCV和SeetaFace的路径配置是否正确
   - 确认您安装了所有必要的依赖项

2. **运行时错误**
   - 确认摄像头连接正常并被系统识别
   - 检查服务器和客户端的网络连接

3. **人脸识别问题**
   - 确保光线充足
   - 尝试调整摄像头位置和角度

### RK3568相关问题

1. **交叉编译问题**
   - 确保使用了正确的工具链版本
   - 检查库的路径配置
   - 常见错误：
     ```
     # 错误：找不到-lGL
     # 解决方法：
     sudo apt-get install libgl1-mesa-dev
     ```

2. **设备资源不足**
   - 监控CPU和内存使用：
     ```bash
     top
     ```
   - 优化应用程序设置，降低分辨率或帧率
   - 关闭不必要的后台程序

3. **摄像头问题**
   - 检查设备是否被正确识别：
     ```bash
     ls -l /dev/video*
     v4l2-ctl --list-formats-ext --device /dev/video0
     ```
   - 尝试不同的摄像头参数：
     ```bash
     v4l2-ctl --set-fmt-video=width=640,height=480,pixelformat=YUYV --device /dev/video0
     ```

4. **显示问题**
   - 检查是否正确安装了Qt的EGLFS或LinuxFB插件
   - 设置正确的环境变量：
     ```bash
     export QT_QPA_PLATFORM=eglfs
     # 或
     export QT_QPA_PLATFORM=linuxfb
     ```

5. **性能优化**
   - 使用进程优先级提高应用性能：
     ```bash
     nice -n -10 ./FaceAttendance
     ```
   - 考虑使用OpenCV的优化编译选项，启用NEON指令集
   - 减小模型大小或使用轻量级检测器

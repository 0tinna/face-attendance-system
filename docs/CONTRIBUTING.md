# 贡献指南

感谢您对人脸考勤系统项目的关注！我们欢迎任何形式的贡献，包括但不限于功能改进、bug修复、文档编写等。本指南将帮助您了解如何参与到项目中来。

## 开发环境设置

在开始贡献代码前，请根据您的目标平台设置相应的开发环境：

### PC开发环境设置

1. **安装Qt 5.14.2或更高版本**

2. **安装OpenCV 4.5.2或更高版本**

3. **安装SeetaFace引擎**

4. **克隆项目仓库**
   
   ```bash
   git clone https://github.com/0tinna/face-attendance-system.git
   cd face-attendance-system
   ```

5. **使用Qt Creator打开项目**
   
   - 打开AttendanceServer.pro和FaceAttendance.pro

### RK3568开发环境设置

如果您希望为RK3568平台贡献代码，需要额外设置：

1. **RK3568工具链设置**
   
   ```bash
   # 下载RK3568 SDK
   git clone https://github.com/rockchip-linux/buildroot.git
   cd buildroot
   
   # 配置工具链
   export CROSS_COMPILE=/path/to/your/toolchain/bin/aarch64-linux-gnu-
   export ARCH=arm64
   ```

2. **交叉编译Qt库**
   
   - 按照[Qt for Embedded Linux](https://doc.qt.io/qt-5/embedded-linux.html)的指南设置交叉编译环境
   - 确保指定了正确的设备规格和工具链

3. **交叉编译OpenCV**
   
   ```bash
   git clone https://github.com/opencv/opencv.git
   cd opencv
   mkdir build && cd build
   cmake -D CMAKE_BUILD_TYPE=RELEASE \
         -D WITH_IPP=OFF \
         -D WITH_OPENCL=OFF \
         -D CMAKE_TOOLCHAIN_FILE=../platforms/linux/aarch64-gnu.toolchain.cmake \
         -D CMAKE_INSTALL_PREFIX=/path/to/install/opencv-arm ..
   make -j4
   make install
   ```

4. **设置Qt Creator交叉编译环境**
   
   - 添加RK3568自定义工具包(Kit)
   - 配置交叉编译器和Qt版本
   - 设置目标部署设备

## 代码风格

为保持代码一致性，请遵循以下代码风格指南：

1. **命名约定**
   
   - 类名：使用大驼峰命名法（如`AttendanceWin`、`QFaceObject`）
   - 方法和变量：使用小驼峰命名法（如`face_register`、`employeeID`）
   - 常量：全大写，下划线分隔（如`MAX_FACE_COUNT`）

2. **代码格式**
   
   - 使用4个空格作为缩进（不使用Tab）
   - 大括号放在同一行
   - 每个文件末尾留一个空行

3. **注释**
   
   - 为类、方法和复杂逻辑添加清晰的注释
   - 使用中文或英文注释均可，但保持统一

## RK3568开发特定指南

开发RK3568版本的考勤终端时，请注意以下事项：

1. **性能优化**
   
   - 考虑RK3568的硬件限制（内存、CPU性能）
   - 优化图像处理算法，尽量使用OpenCV的NEON优化版本
   - 避免过度使用复杂的UI效果，以保持流畅运行
   - 测试不同分辨率下的性能表现

2. **UI适配**
   
   - 确保UI适合触摸屏操作，按钮和控件大小合适
   - 考虑在不同屏幕分辨率下的显示效果
   - 支持横屏和竖屏显示模式

3. **硬件测试**
   
   - 测试不同类型摄像头的兼容性（USB、CSI接口）
   - 验证在不同光线条件下的人脸检测效果
   - 测试长时间运行的稳定性，留意内存泄漏问题

4. **部署测试**
   
   - 测试应用程序的自启动和恢复机制
   - 验证断电恢复后的数据一致性
   - 确保网络波动时能够正常工作（离线模式）

## 贡献流程

1. **创建Issue**
   
   - 贡献前先创建Issue描述您要解决的问题或添加的功能
   - 等待维护者确认，以避免重复工作

2. **Fork仓库**
   
   - 在GitHub上Fork项目仓库到您的账号下

3. **创建分支**
   
   - 基于main分支创建新的功能分支
     
     ```bash
     git checkout -b feature/your-feature-name
     # 或
     git checkout -b bugfix/bug-being-fixed
     ```

4. **编写代码**
   
   - 实现您的功能或修复bug
   - 添加必要的测试
   - 确保现有测试通过

5. **提交更改**
   
   ```bash
   git add .
   git commit -m "描述您的更改"
   ```
   
   - 提交信息应简洁明了，说明此次提交的主要内容

6. **推送到您的Fork**
   
   ```bash
   git push origin feature/your-feature-name
   ```

7. **创建Pull Request**
   
   - 在GitHub上从您的分支创建Pull Request到原仓库的main分支
   - 详细描述您的更改内容和解决的问题
   - 引用相关Issue编号

8. **代码审核**
   
   - 维护者会审核您的代码
   - 根据反馈进行必要的修改

9. **合并代码**
   
   - 一旦审核通过，维护者会将您的代码合并到主分支

## 报告Bug

如果您发现了bug但暂时无法修复，请通过以下步骤报告：

1. 检查Issue列表，确认该bug尚未被报告
2. 创建新Issue，添加"bug"标签
3. 详细描述bug的复现步骤、预期行为和实际行为
4. 如可能，添加截图、错误日志或相关信息

## 功能请求

如果您有新功能的想法：

1. 创建新Issue，添加"enhancement"标签
2. 详细描述该功能的作用和实现思路
3. 如可能，提供UI设计或流程图

## 文档贡献

文档改进同样重要：

1. 对于小的修正（如拼写错误），可直接提交PR
2. 对于较大的改动（如添加新章节），请先创建Issue讨论

## 行为准则

参与本项目即表示您同意遵守以下行为准则：

- 尊重所有参与者，不发表攻击性言论
- 接受建设性批评和反馈
- 专注于项目改进，避免无关讨论

## 许可证

本项目采用MIT许可证。您贡献的代码将同样遵循该许可证。

感谢您的贡献！

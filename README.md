# 基于Qt和SeetaFace的人脸考勤系统 (RK3568)

这是一个使用Qt框架和SeetaFace引擎开发的人脸考勤系统，支持人脸识别、考勤记录管理和人员信息管理。本系统特别优化适配了瑞芯微RK3568开发板，可作为独立的考勤终端部署。

## 项目结构

- `src/AttendanceServer`：考勤服务器端（Qt应用程序）
- `src/FaceAttendance`：考勤客户端（Qt应用程序）
- `docs/`：项目文档

## 技术栈

- Qt 5.14.2 (GUI框架)
- OpenCV 4.5.2 (计算机视觉库)
- SeetaFace (人脸识别引擎)
- SQLite (数据库)

## 功能特点

- 人脸检测与识别
- 员工信息管理
- 考勤记录管理
- 基于C/S架构的网络通信
- 支持RK3568嵌入式开发板部署
- 针对ARM架构优化的人脸检测算法

## 安装与部署

请参考 [安装指南](docs/installation.md) 获取完整的安装和部署说明。

## 使用说明

请参考 [用户手册](docs/user-manual.md) 获取详细的使用说明。

## RK3568开发板指南

- [RK3568故障排查指南](docs/rk3568-troubleshooting.md) - 解决RK3568平台常见问题
- 您可以在用户手册中找到关于RK3568终端操作的详细说明

## 系统架构

请参考 [系统设计文档](docs/design.md) 了解系统架构和设计。

## 开发环境

### PC端开发环境
- Windows/Linux
- Qt 5.14.2
- MinGW 64-bit/GCC
- OpenCV 4.5.2
- SeetaFace

### 嵌入式开发环境
- 瑞芯微RK3568开发板
- 嵌入式Linux (Debian/Buildroot)
- 交叉编译工具链
- Qt 5.14.2 (ARM版本)
- OpenCV 4.5.2 (ARM版本，启用NEON优化)

## 许可证

本项目采用 [MIT License](LICENSE) 许可证。

## 联系方式

如有问题，请联系项目维护者。

## GitHub 仓库上传指南

如果您需要将此项目上传到 GitHub，请参考 [GitHub 上传指南](docs/github-upload-guide.md)。

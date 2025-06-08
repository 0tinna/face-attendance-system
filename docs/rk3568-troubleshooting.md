# RK3568人脸考勤终端故障排查指南

本文档提供RK3568人脸考勤终端常见问题的解决方法，适用于开发人员和终端使用者。

## 启动问题

### 设备不能启动

1. **检查电源**
   - 确认电源适配器稳定提供12V/2A电源
   - 尝试更换电源适配器或电源线
   
2. **检查系统**
   - 如果有串口调试器，连接查看启动日志
   - 观察开发板上的LED指示灯状态

### 应用程序不自动启动

1. **检查自启动配置**
   ```bash
   ls -la ~/.config/autostart/
   cat ~/.config/autostart/face-attendance.desktop
   ```

2. **手动启动应用程序**
   ```bash
   cd /path/to/FaceAttendance
   ./FaceAttendance
   ```
   观察是否有错误消息输出

## 摄像头问题

### 摄像头不工作

1. **检查设备是否被识别**
   ```bash
   ls -l /dev/video*
   v4l2-ctl --list-devices
   ```

2. **测试摄像头**
   ```bash
   v4l2-ctl --device=/dev/video0 --stream-mmap --stream-count=100
   # 或使用GStreamer测试
   gst-launch-1.0 v4l2src device=/dev/video0 ! videoconvert ! ximagesink
   ```

3. **查看摄像头权限**
   ```bash
   ls -la /dev/video*
   # 确保当前用户在video组中
   groups
   # 如不在，添加用户到video组
   sudo usermod -a -G video $USER
   ```

### 人脸检测不准确

1. **检查光线条件**
   - 确保光线充足且均匀
   - 避免强光直射摄像头
   
2. **调整摄像头参数**
   ```bash
   # 调整亮度/对比度
   v4l2-ctl -d /dev/video0 --set-ctrl=brightness=128
   v4l2-ctl -d /dev/video0 --set-ctrl=contrast=128
   ```

3. **查看图像质量**
   - 使用工具查看实时图像质量
   - 调整摄像头位置和角度

## 网络问题

### 无法连接服务器

1. **检查网络连接**
   ```bash
   # 检查IP地址
   ip addr show
   
   # 测试网络连通性
   ping 192.168.1.x  # 替换为您的服务器IP
   ```

2. **检查防火墙设置**
   ```bash
   # 查看防火墙规则
   sudo iptables -L
   
   # 测试端口连通性
   nc -zv 192.168.1.x 9999  # 替换为您的服务器IP和端口
   ```

3. **查看应用日志**
   ```bash
   tail -f /path/to/app/logs/faceattendance.log
   ```

## 性能问题

### 系统运行缓慢

1. **检查系统资源使用情况**
   ```bash
   top
   free -h
   df -h
   ```

2. **查看温度**
   ```bash
   cat /sys/class/thermal/thermal_zone0/temp
   ```
   
3. **调整CPU频率**
   ```bash
   # 查看当前CPU频率
   cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq
   
   # 设置性能模式
   echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
   ```

### 内存泄漏

1. **监控内存使用**
   ```bash
   watch -n 1 free -m
   ```

2. **使用valgrind检测**
   ```bash
   # 安装valgrind
   sudo apt-get install valgrind
   
   # 运行检测
   valgrind --leak-check=full ./FaceAttendance
   ```

## 存储问题

### 数据库错误

1. **检查数据库文件权限**
   ```bash
   ls -la /path/to/your/database.db
   ```

2. **检查存储空间**
   ```bash
   df -h
   ```

3. **验证数据库完整性**
   ```bash
   sqlite3 /path/to/your/database.db "PRAGMA integrity_check;"
   ```

### 日志文件过大

1. **查看日志文件大小**
   ```bash
   du -sh /path/to/logs/*
   ```

2. **设置日志轮转**
   ```bash
   # 创建logrotate配置文件
   sudo nano /etc/logrotate.d/faceattendance
   
   # 添加配置
   /path/to/logs/*.log {
       rotate 7
       daily
       compress
       missingok
       notifempty
   }
   ```

## 软件更新

### 应用程序更新

1. **备份重要数据**
   ```bash
   cp /path/to/database.db /path/to/backup/
   ```

2. **更新应用程序**
   ```bash
   # 停止旧应用
   killall FaceAttendance
   
   # 复制新版本
   cp /path/to/new/FaceAttendance /path/to/app/
   chmod +x /path/to/app/FaceAttendance
   
   # 启动新应用
   /path/to/app/FaceAttendance &
   ```

### 系统固件更新

请参考RK3568开发板的官方文档进行系统固件更新，或联系技术支持。

## 联系支持

如果您遇到无法解决的问题，请通过以下方式联系技术支持：

- 提交GitHub Issue
- 发送问题描述至支持邮箱

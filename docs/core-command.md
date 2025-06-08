```bash
export ARCH=arm64  && export ACROSS_COMPILE=/root/rk3568_linux_sdk/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gun-
```

```bash
/opt/atk-dlrk356x-toolchain/usr/bin/qmake
```

```bash
make clean && make -j 4
```

```bash
adb push FaceAttendance /env
```

```
netsh advfirewall firewall add rule name="Allow ICMPv4-In" protocol=icmpv4:8,any dir=in action=allow
```

| 开发板和电脑连接同一个网络                |
| ---------------------------- |
| 通过命令让开发板ping通电脑              |
| 电脑运行server                   |
| 开发板运行faceattendance          |
| 注意事项：关闭VPN，不要打开tcp工具测试，会占用端口 |

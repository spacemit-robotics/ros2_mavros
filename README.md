# MAVROS for RISC-V

这是面向 RISC-V 的 MAVROS，已在 SpacemiT K3（`riscv64`、ROS 2 Humble）原生编译并通过软件通信验证。

它通过 MAVLink 连接飞控（如 PX4），保留原有 MAVROS 话题；IMU、位置和遥控等话题使用 Fast DDS 直连。启用相应插件后，也能把应用提供的视觉位姿转发给飞控。

在装有 ROS 2 Humble 的目标设备上操作。以下命令在 SDK 根目录（包含 `build/envsetup.sh` 的目录）执行。

## 1. 编译 MAVROS

如果 SDK 已经带有编译好的 MAVROS，跳过这一步。
编译时，SDK 会提示安装缺少的 apt 依赖，按提示确认即可。

```bash
source build/envsetup.sh
./build/build.sh package middleware/ros2/mavros/mavros_extras --with-deps
```

## 2. 启动

启动前需要准备 GeographicLib 的 `egm96-5` 数据（缺少时使用 [数据安装脚本](mavros/scripts/install_geographiclib_datasets.sh)）。

按实际连接方式选择 `fcu_url`，替换示例中的设备路径和波特率：

| 连接方式 | `fcu_url` 示例 |
| --- | --- |
| USB 串口 | `/dev/ttyACM0:57600` |
| 板载 UART | `/dev/ttyS5:921600` |

两种方式用同一个命令，只改 `fcu_url:=` 后面的值：

```bash
source build/envsetup.sh
sros2_setup
ros2 run mavros mavros_node --ros-args -p 'fcu_url:=/dev/ttyACM0:57600' -p 'plugin_allowlist:=[sys_status, imu, rc_io]'
```

这个示例开启飞控状态、IMU 和遥控输入。按 `Ctrl-C` 停止。
其他写法见 [连接地址格式](libmavconn/README.md#connection-url)。

## 3. 确认收到数据

另开终端，运行 `source build/envsetup.sh` 和 `sros2_setup`。保持和启动终端相同的 `ROS_DOMAIN_ID`，再逐条执行：

```bash
ros2 topic echo /mavros/state --qos-durability transient_local
ros2 topic echo /mavros/imu/data_raw --qos-reliability best_effort --once
```

看到 `connected: true` 后按 `Ctrl-C`，再执行第二条。收到一条 IMU 消息即为通过。
如果一直不连接，先检查串口路径、读写权限、波特率和串口是否被占用。

[上游说明](UPSTREAM.md) · [许可证](LICENSE.md)

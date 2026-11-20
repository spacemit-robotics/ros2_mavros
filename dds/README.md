# MAVROS 内部 Fast DDS 适配

此目录随 MAVROS 一起交付，不是 SDK 公共依赖。按 ROS 消息包组织适配；不同话题名复用同一个类型适配。ROS 包名和 CMake target 保留原名。

```text
core/                    # fastdds_ros_core：域、话题、QoS 和端点生命周期
rosidl/                  # Fast DDS 与 ROS 生成的 CDR 类型支持之间的桥接
geometry_msgs/
├── pose_stamped/
├── twist/
└── twist_stamped/
sensor_msgs/
├── imu/                 # MAVROS 的 IMU wire 实现，生成类型仅在库内可见
└── nav_sat_fix/
mavros_msgs_dds/         # MAVROS-specific RCIn
```

`core` 不依赖 ROS 消息包。后续添加消息类型时，在对应消息包下接入 ROS 生成的 Fast RTPS 类型支持，不维护第二份序列化。MAVROS 插件负责配置话题名，不在 `core` 注册业务话题。

```cmake
find_package(fastdds_ros_sensor_msgs REQUIRED)
target_link_libraries(your_target PRIVATE fastdds_ros_sensor_msgs::imu)
```

一个进程内，同一 DDS 域的多个 MAVROS 话题使用同一个 `Context`。每个话题仍通过独立端点收发。

```cpp
#include <fastdds_ros/context.hpp>
#include <fastdds_ros/imu.hpp>

auto context = fastdds_ros::Context::create(domain_id);
fastdds_ros::ImuPublisher camera_imu;
fastdds_ros::ImuPublisher flight_imu;
if (!context || !camera_imu.init("rt/camera/imu", context) ||
    !flight_imu.init("rt/flight/imu", context)) {
    // Report the setup error; do not publish.
}
```

`init` 保持原始 DDS 话题名语义：`rt/imu` 对应 ROS 2 的 `/imu`。IMU 默认 QoS 保持 Best Effort、Volatile、Keep Last 10，传输为共享内存和 UDPv4。订阅回调在 DDS 接收线程执行；不要在回调里销毁收发端。单话题客户端仍可直接传域 ID，由适配器创建私有 `Context`。

`fastdds_ros_sensor_msgs::imu` 对应 `libmavros_dds_imu.so`，不占用 SDK IMU 的 `libperipherals_imu_node_lib.so` 库名，也不安装 SDK IMU 的公开包装头。SDK UART IMU 继续在 `middleware/ros2/peripherals/imu` 内独立构建。

启用 `BUILD_TESTING=ON` 后，`imu_ros2_interop` 在两个话题上验证合成 IMU 消息与 ROS 2 Fast DDS RMW 双向互通、话题不串数据、两个发布端共享同一个 DDS participant，无需接入硬件。

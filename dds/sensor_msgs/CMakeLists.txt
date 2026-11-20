cmake_minimum_required(VERSION 3.10)
project(fastdds_ros_sensor_msgs LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)
find_package(fastdds_ros_core REQUIRED)
find_package(fastdds_ros_rosidl REQUIRED)
find_package(sensor_msgs REQUIRED)

if(BUILD_TESTING)
  find_package(rclcpp REQUIRED)
  find_package(Threads REQUIRED)
  enable_testing()
endif()

add_subdirectory(imu)
add_library(nav_sat_fix INTERFACE)
target_include_directories(nav_sat_fix INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/nav_sat_fix/include>"
  "$<INSTALL_INTERFACE:include>")
target_link_libraries(nav_sat_fix INTERFACE fastdds_ros_rosidl::rosidl
  sensor_msgs::sensor_msgs__rosidl_typesupport_fastrtps_cpp)
install(TARGETS nav_sat_fix EXPORT fastdds_ros_sensor_msgs_targets)
install(DIRECTORY nav_sat_fix/include/ DESTINATION include)

install(FILES ../LICENSE DESTINATION share/${PROJECT_NAME})
ament_export_targets(fastdds_ros_sensor_msgs_targets HAS_LIBRARY_TARGET)
ament_export_dependencies(fastcdr fastrtps fastdds_ros_core fastdds_ros_rosidl sensor_msgs)
ament_package()

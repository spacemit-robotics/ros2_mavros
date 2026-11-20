cmake_minimum_required(VERSION 3.10)
project(fastdds_ros_mavros_msgs LANGUAGES CXX)
find_package(ament_cmake REQUIRED)
find_package(fastdds_ros_rosidl REQUIRED)
find_package(mavros_msgs REQUIRED)

add_library(rc_in INTERFACE)
target_link_libraries(rc_in INTERFACE fastdds_ros_rosidl::rosidl
  mavros_msgs::mavros_msgs__rosidl_typesupport_fastrtps_cpp)
foreach(message IN ITEMS rc_in)
  target_include_directories(rc_in INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${message}/include>")
  install(DIRECTORY ${message}/include/ DESTINATION include)
endforeach()
target_include_directories(rc_in INTERFACE "$<INSTALL_INTERFACE:include>")

install(TARGETS rc_in EXPORT fastdds_ros_mavros_msgs_targets)
install(FILES LICENSE DESTINATION share/${PROJECT_NAME})
ament_export_targets(fastdds_ros_mavros_msgs_targets)
ament_export_dependencies(fastdds_ros_rosidl mavros_msgs)

if(BUILD_TESTING)
  find_package(rclcpp REQUIRED)
  find_package(fastdds_ros_geometry_msgs REQUIRED)
  find_package(fastdds_ros_sensor_msgs REQUIRED)
  enable_testing()
  add_executable(test_mavros_dds_interop test/test_mavros_dds_interop.cpp)
  target_compile_features(test_mavros_dds_interop PRIVATE cxx_std_17)
  target_link_libraries(test_mavros_dds_interop PRIVATE rc_in rclcpp::rclcpp
    fastdds_ros_geometry_msgs::geometry_msgs fastdds_ros_sensor_msgs::nav_sat_fix
    geometry_msgs::geometry_msgs__rosidl_typesupport_cpp
    sensor_msgs::sensor_msgs__rosidl_typesupport_cpp
    mavros_msgs::mavros_msgs__rosidl_typesupport_cpp)
  add_test(NAME mavros_dds_interop COMMAND test_mavros_dds_interop)
  set_tests_properties(mavros_dds_interop PROPERTIES TIMEOUT 35
    ENVIRONMENT "RMW_IMPLEMENTATION=rmw_fastrtps_cpp")
endif()

ament_package()

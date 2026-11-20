cmake_minimum_required(VERSION 3.10)
project(fastdds_ros_rosidl LANGUAGES CXX)
find_package(ament_cmake REQUIRED)
find_package(fastdds_ros_core REQUIRED)
find_package(fastcdr REQUIRED)
find_package(rosidl_typesupport_fastrtps_cpp REQUIRED)

add_library(rosidl INTERFACE)
target_include_directories(rosidl INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>")
target_link_libraries(rosidl INTERFACE fastdds_ros_core::core fastcdr
  rosidl_typesupport_fastrtps_cpp::rosidl_typesupport_fastrtps_cpp)

install(TARGETS rosidl EXPORT fastdds_ros_rosidl_targets)
install(DIRECTORY include/ DESTINATION include)
install(FILES ../LICENSE DESTINATION share/${PROJECT_NAME})
ament_export_targets(fastdds_ros_rosidl_targets)
ament_export_dependencies(fastdds_ros_core fastcdr rosidl_typesupport_fastrtps_cpp)
ament_package()

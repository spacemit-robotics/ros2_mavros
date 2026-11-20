cmake_minimum_required(VERSION 3.10)
project(fastdds_ros_geometry_msgs LANGUAGES CXX)
find_package(ament_cmake REQUIRED)
find_package(fastdds_ros_rosidl REQUIRED)
find_package(geometry_msgs REQUIRED)

add_library(geometry_msgs INTERFACE)
target_link_libraries(geometry_msgs INTERFACE fastdds_ros_rosidl::rosidl
  geometry_msgs::geometry_msgs__rosidl_typesupport_fastrtps_cpp)
foreach(message IN ITEMS pose_stamped twist twist_stamped)
  target_include_directories(geometry_msgs INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${message}/include>")
  install(DIRECTORY ${message}/include/ DESTINATION include)
endforeach()
target_include_directories(geometry_msgs INTERFACE "$<INSTALL_INTERFACE:include>")

install(TARGETS geometry_msgs EXPORT fastdds_ros_geometry_msgs_targets)
install(FILES ../LICENSE DESTINATION share/${PROJECT_NAME})
ament_export_targets(fastdds_ros_geometry_msgs_targets)
ament_export_dependencies(fastdds_ros_rosidl geometry_msgs)
ament_package()

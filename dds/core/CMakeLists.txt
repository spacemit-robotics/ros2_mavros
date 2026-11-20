cmake_minimum_required(VERSION 3.10)
project(fastdds_ros_core LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(fastrtps REQUIRED)
find_package(Threads REQUIRED)

add_library(fastdds_ros_core SHARED src/context.cpp)
set_target_properties(fastdds_ros_core PROPERTIES EXPORT_NAME core CXX_EXTENSIONS OFF)
target_compile_features(fastdds_ros_core PUBLIC cxx_std_14)
target_include_directories(fastdds_ros_core PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>")
target_link_libraries(fastdds_ros_core PUBLIC fastrtps PRIVATE Threads::Threads)

install(TARGETS fastdds_ros_core EXPORT fastdds_ros_core_targets LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES ../LICENSE DESTINATION share/${PROJECT_NAME})
ament_export_targets(fastdds_ros_core_targets HAS_LIBRARY_TARGET)
ament_export_dependencies(fastrtps)
ament_package()

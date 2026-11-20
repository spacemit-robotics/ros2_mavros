add_library(mavros_imu_wire OBJECT
  wire/Header.cxx
  wire/HeaderPubSubTypes.cxx
  wire/Imu.cxx
  wire/ImuPubSubTypes.cxx
  wire/Quaternion.cxx
  wire/QuaternionPubSubTypes.cxx
  wire/Time.cxx
  wire/TimePubSubTypes.cxx
  wire/Vector3.cxx
  wire/Vector3PubSubTypes.cxx)
# The SDK IMU owns a separate copy of the generated wire types. Keep these
# symbols private even if a process loads both packages.
set_target_properties(mavros_imu_wire PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  CXX_EXTENSIONS OFF)
target_compile_features(mavros_imu_wire PRIVATE cxx_std_14)
target_link_libraries(mavros_imu_wire PRIVATE fastcdr fastrtps)

add_library(fastdds_imu SHARED src/imu.cpp $<TARGET_OBJECTS:mavros_imu_wire>)
set_target_properties(fastdds_imu PROPERTIES
  EXPORT_NAME imu
  OUTPUT_NAME mavros_dds_imu
  CXX_EXTENSIONS OFF)
target_compile_features(fastdds_imu PUBLIC cxx_std_14)
target_include_directories(fastdds_imu PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/wire")
target_link_libraries(fastdds_imu PUBLIC fastdds_ros_core::core PRIVATE fastcdr fastrtps)
ament_target_dependencies(fastdds_imu PUBLIC sensor_msgs)

install(TARGETS fastdds_imu EXPORT fastdds_ros_sensor_msgs_targets
  LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTING)
  add_executable(test_imu_interop tests/test_imu_interop.cpp)
  target_link_libraries(test_imu_interop fastdds_imu Threads::Threads)
  ament_target_dependencies(test_imu_interop rclcpp sensor_msgs)
  add_test(NAME imu_ros2_interop COMMAND test_imu_interop)
  set_tests_properties(imu_ros2_interop PROPERTIES TIMEOUT 20)
endif()

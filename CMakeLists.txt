cmake_minimum_required(VERSION 3.8)
project(viskerobot)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(xarm_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(action_msgs REQUIRED)
find_package(control_msgs REQUIRED)

set(dependencies "rclcpp" "rclcpp_action" "rclcpp_components" "std_msgs" "geometry_msgs" "xarm_msgs" "control_msgs")

add_executable(grip_w_subscriber src/gripper_action_subscriber.cpp)
ament_target_dependencies(grip_w_subscriber
  ${dependencies}
)

add_executable(throwing_pub src/throwing_pub.cpp)
ament_target_dependencies(throwing_pub
  ${dependencies}
)

add_executable(throwing_pub2 src/throwing_pub2.cpp)
ament_target_dependencies(throwing_pub2
  ${dependencies}
)

add_executable(throwing_math src/throwing_pub_math.cpp)
ament_target_dependencies(throwing_math
  ${dependencies}
)

install(TARGETS
  grip_w_subscriber
  throwing_pub
  throwing_pub2
  throwing_math
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()

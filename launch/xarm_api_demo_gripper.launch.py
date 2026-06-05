#!/usr/bin/env python3

from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    dof = LaunchConfiguration('dof')
    robot_type = LaunchConfiguration('robot_type', default='xarm')

    throwing_main_node = Node(
        name='throwing_pub_math',
        package='viskerobot_demod',
        executable='throwing_math',
        output='screen',
        parameters=[],
    )

    throwing_gripper_node = Node(
        name='gripper_action_subscriber',
        package='viskerobot_demod',
        executable='grip_w_subscriber',
        output='screen',
        parameters=[], #yeah don't think we need those here either
    )

    return LaunchDescription([
        throwing_main_node,
        throwing_gripper_node
    ])

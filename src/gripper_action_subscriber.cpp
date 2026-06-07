#include <functional>
#include <future>
#include <memory>
#include <string>
#include <sstream>

#include "control_msgs/action/gripper_command.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"

//message type used for sending desired gripper position
#include "std_msgs/msg/float32.hpp"

class GripperMoveClient : public rclcpp::Node
{
public: //defines variables to simplify text in further commands
  using gripper_command = control_msgs::action::GripperCommand;
  using goal_handler = rclcpp_action::ClientGoalHandle<gripper_command>;

  GripperMoveClient()
  : Node("gripper_move_client")
  { //defines the action client for gripper control
    this->gripper_action_client = rclcpp_action::create_client<gripper_command>(this, "/xarm_gripper/gripper_action");
      
    auto send_goal = [this](const std_msgs::msg::Float32::UniquePtr msg) -> void
    { //shuts the node down if action server does not become available 
      if (!this->gripper_action_client->wait_for_action_server(std::chrono::seconds(5))) {
        RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
        rclcpp::shutdown();
      }

      //defines the type of the goal message sent to action server
      auto goal_msg = gripper_command::Goal();
      goal_msg.command.position = msg->data; //defines the received message as the goal position
      goal_msg.command.max_effort = 0.0; //gripper G1 does not take max effort as a variable, but gripper G2 does

      RCLCPP_INFO(this->get_logger(), "Sending goal");

      //defines additional goal sending options
      auto send_goal_options = rclcpp_action::Client<gripper_command>::SendGoalOptions();
    
      //defines how to handle action results 
      //in this case they will be displayed in the log
      send_goal_options.result_callback = [this](const goal_handler::WrappedResult & result) {
        switch (result.code) {
          case rclcpp_action::ResultCode::SUCCEEDED:
            RCLCPP_INFO(this->get_logger(), "Goal succeeded");
            break;
          case rclcpp_action::ResultCode::ABORTED:
            RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
            break;
          case rclcpp_action::ResultCode::CANCELED:
            RCLCPP_ERROR(this->get_logger(), "Goal was canceled");
            break;
          default:
            RCLCPP_ERROR(this->get_logger(), "Unknown result code");
            break;
        }
      };

      //sends the gripper goal position to the robot controller
      this->gripper_action_client->async_send_goal(goal_msg, send_goal_options);
    };

    //subscriber for receiving desired gripper positions
    gripper_subscription = this->create_subscription<std_msgs::msg::Float32>("gripper_moving_call", 10, send_goal);
  }

private:
  //defines types for variables gripper_action_client and gripper_subscription
  rclcpp_action::Client<gripper_command>::SharedPtr gripper_action_client;
  rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr gripper_subscription;
};

//handles node creation, spinning and stopping
int main(int argc, char ** argv){
  rclcpp::init(argc, argv);
  auto action_client_node = std::make_shared<GripperMoveClient>();
  rclcpp::spin(action_client_node);
  RCLCPP_INFO(action_client_node->get_logger(), "Stopping node");
  rclcpp::shutdown();
  return 0;
}

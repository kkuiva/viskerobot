#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <xarm_msgs/msg/robot_msg.hpp>
 
#include <xarm_msgs/srv/set_int16.hpp>
#include <xarm_msgs/srv/set_int16_by_id.hpp>
#include <xarm_msgs/srv/move_joint.hpp>
#include <xarm_msgs/srv/move_cartesian.hpp>

//message type used for sending desired gripper position
#include "std_msgs/msg/float32.hpp"

using namespace std::chrono_literals; 

#define PI 3.14159265358979323846

class XArmApiDemo : public rclcpp::Node
{
public:
  XArmApiDemo()
  : Node("xarm_api_demo")
  { //declaring robot model and dof
    hw_ns = this->declare_parameter<std::string>("hw_ns", "xarm");
    dof = this->declare_parameter<int>("dof", 7);

    //service clients used in controlling the robot:
    //enabling the robot to move, setting mode and state, sending joint and cartesian movement commands
    motion_enable_client = this->create_client<xarm_msgs::srv::SetInt16ById>("/" + hw_ns + "/motion_enable");
    set_mode_client = this->create_client<xarm_msgs::srv::SetInt16>("/" + hw_ns + "/set_mode");
    set_state_client = this->create_client<xarm_msgs::srv::SetInt16>("/" + hw_ns + "/set_state");
    move_joint_client = this->create_client<xarm_msgs::srv::MoveJoint>("/" + hw_ns + "/set_servo_angle");
    move_cartesian_client = this->create_client<xarm_msgs::srv::MoveCartesian>("/" + hw_ns + "/set_position");

    //robot poses used in the program:
    //home position, pick approach, throwing initial and final positions
    //the angle needs to be in rad, so the format x * PI/180 can be used to insert the angle, where x is the angle in deg
    joint_pos_1 = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    joint_pos_2 = {30.2 * PI/180.0, 22 * PI/180.0, 2.1 * PI/180.0, 40.1 * PI/180.0, -2.6 * PI/180.0, 18.1 * PI/180.0, 34.7 * PI/180.0 };
    joint_pos_3 = { 0.0, 0.0, 0.0, 0.2, 0.0, 0.7, 0.0 };
    joint_pos_4 = { 0.0, 0.0, 0.0, PI/2, 0.0, 0.0, 0.0 };

    //robot position used in the program, which is the picking position (mm, mm, mm, rad, rad, rad) 
    //important: the tool coordinates are 180 deg rotated around x axis, so the roll value needs to be PI for "normal" position
    cart_pos_1 = { 348.3, 224.1, 7, PI, 0.0, 0.0 };

    //publisher for the desired gripper position
    gripper_publisher = this->create_publisher<std_msgs::msg::Float32>("gripper_moving_call", 10);
  }

  //initializing the robot
  bool initialize_robot()
  {
    RCLCPP_INFO(get_logger(), "Waiting for xArm API services...");

    //waits for all services to be ready
    if (!wait_for_client(motion_enable_client) ||
        !wait_for_client(set_mode_client) ||
        !wait_for_client(set_state_client) ||
        !wait_for_client(move_joint_client) ||
        !wait_for_client(move_cartesian_client))
    {
      return false;
    }

    //enables all joints to move
    auto enable_req = std::make_shared<xarm_msgs::srv::SetInt16ById::Request>();
    enable_req->id = 8;
    enable_req->data = 1;
    if (!call_simple(motion_enable_client, enable_req, "motion_enable")) {
      return false;
    }

    //sets the mode as 0
    auto mode_req = std::make_shared<xarm_msgs::srv::SetInt16::Request>();
    mode_req->data = 0;
    if (!call_simple(set_mode_client, mode_req, "set_mode")) {
      return false;
    }

    //sets the state as 0
    auto state_req = std::make_shared<xarm_msgs::srv::SetInt16::Request>();
    state_req->data = 0;
    if (!call_simple(set_state_client, state_req, "set_state")) {
      return false;
    }

    RCLCPP_INFO(get_logger(), "Robot initialized.");
    return true;
  }

  //publishing the gripper's desired position
  void gripper_callback(float gripper_data){
    auto message = std_msgs::msg::Float32();
    message.data = gripper_data;
    RCLCPP_INFO(get_logger(), "Publishing gripper position");
    gripper_publisher->publish(message);
    };

  //main cycle of the program
  bool run_demo()
  {  
    //home position
    if (!move_joint(joint_pos_1, 1, 10.0, true)) {
      return false;
    }

    //moving above the cube
    if (!move_joint(joint_pos_2, 1, 20.0, true)) {
      return false;
    }
    
    //opening the gripper
    gripper_callback(0.2);
    sleep(2);

    //moving into the picking position
    if (!move_cartesian(cart_pos_1, 120.0, 1000.0, true)) {
      return false;
    }

    //closing the gripper
    gripper_callback(0.57);
    sleep(2);

    //lifting the cube straight up
    if (!move_joint(joint_pos_2, 120.0, 1000.0, true)) {
      return false;
    }

    //moving to the start position of the throwing move
    if (!move_joint(joint_pos_3, 1, 15.0, true)) {
      return false;
    }

    //throwing move (moving to the throwing final position)
    if (!move_joint(joint_pos_4, 6, 100.0, false)) {
      return false;
    }
    
    //waiting to time the gripper
    rclcpp::sleep_for(std::chrono::milliseconds(450));
    //opening the gripper
    gripper_callback(0.2);
    
    //moving to home position
     if (!move_joint(joint_pos_1, 0.35, 10.0, false)) {

      return false;
    }

    RCLCPP_INFO(get_logger(), "End of demo");
    return true;
  }

private:
  //function for waiting for services to become available
  template<typename ClientPtrT>
  bool wait_for_client(const ClientPtrT & client)
  {
    while (!client->wait_for_service(1s)) {   
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(get_logger(), "Process was interrupted while waiting for service");
        return false;
      }
      RCLCPP_WARN(get_logger(), "Waiting for service: %s", client->get_service_name());
    }
    return true;
  }

  template<typename ClientPtrT, typename RequestT>
  bool call_simple( //function that sends all service calls
    const ClientPtrT & client,
    const std::shared_ptr<RequestT> & req,
    const std::string & label){
    auto future = client->async_send_request(req);
    auto ret = rclcpp::spin_until_future_complete(shared_from_this(), future);

    if (ret != rclcpp::FutureReturnCode::SUCCESS) {
      RCLCPP_ERROR(get_logger(), "Service call failed: %s", label.c_str());
      return false;
    }

    auto res = future.get();
    if (res->ret != 0) {
      RCLCPP_ERROR(get_logger(), "Service %s returned ret=%d", label.c_str(), res->ret);
      return false;
    }

  return true;
  }

  //function for sending desired robot pose
  bool move_joint(const std::vector<double> & joints, double speed, double acc, bool wait)  
  { //checks that the number of angles given matches number of joints
    if ((int)joints.size() != dof) {
      RCLCPP_ERROR(get_logger(), "Number of joints does not match: expected %d, was given %zu", dof, joints.size());
      return false;
    }
    //defines the type of request and matches the variables given to data fields
    auto req = std::make_shared<xarm_msgs::srv::MoveJoint::Request>();
    req->angles = std::vector<float>(joints.begin(), joints.end());
    req->speed = speed;
    req->acc = acc;
    req->mvtime = 0.0;
    req->wait = wait;
    req->timeout = 0.0;
    req->radius = 20.0;

    RCLCPP_INFO(
      get_logger(),
      "Moving to pose: speed=%.3f acc=%.3f wait=%d", speed, acc, wait);
    //sending the request with the call_simple function
    return call_simple(move_joint_client, req, "set_servo_angle");
  }

  //function for sending desired robot position
  bool move_cartesian(const std::vector<float> & pose, float speed, float acc, bool wait)
  {
    if (pose.size() != 6) {
      RCLCPP_ERROR(get_logger(), "Position must be defined with 6 values");
      return false;
    }
    //defines the type of request and matches the variables given to data fields
    auto req = std::make_shared<xarm_msgs::srv::MoveCartesian::Request>();
    req->pose = pose;  
    req->speed = speed; 
    req->acc = acc;
    req->mvtime = 0.0;
    req->wait = wait;
    req->timeout = 0.0;

    RCLCPP_INFO(
      get_logger(),
      "Moving to position: speed=%.1f acc=%.1f wait=%d", speed, acc, wait);
    //sending the request with the call_simple function
    return call_simple(move_cartesian_client, req, "set_position");
  }

//defining variable types used
private:
  std::string hw_ns;
  int dof;
  
  rclcpp::Client<xarm_msgs::srv::SetInt16ById>::SharedPtr motion_enable_client;
  rclcpp::Client<xarm_msgs::srv::SetInt16>::SharedPtr set_mode_client;
  rclcpp::Client<xarm_msgs::srv::SetInt16>::SharedPtr set_state_client;
  rclcpp::Client<xarm_msgs::srv::MoveJoint>::SharedPtr move_joint_client;
  rclcpp::Client<xarm_msgs::srv::MoveCartesian>::SharedPtr move_cartesian_client;

  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr gripper_publisher;

  std::vector<double> joint_pos_1;
  std::vector<double> joint_pos_2;
  std::vector<double> joint_pos_3;
  std::vector<double> joint_pos_4;

  std::vector<float> cart_pos_1;
};

//main function for running different parts of the program and starting and shutting down the node
int main(int argc, char ** argv){
  rclcpp::init(argc, argv);
  auto node = std::make_shared<XArmApiDemo>();

  //initializing the robot
  if (!node->initialize_robot()) {
    RCLCPP_ERROR(node->get_logger(), "Failed to initialize the robot");
    rclcpp::shutdown();
    return 1;
  }

  //running the demo
  if (!node->run_demo()) {
    RCLCPP_ERROR(node->get_logger(), "Demo failed");
    rclcpp::shutdown();
    return 1;
  }

  rclcpp::shutdown();
  return 0;
}

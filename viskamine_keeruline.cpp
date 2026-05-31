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

//sõnumi tüüp haaratsi soovitud asendi edastamiseks
#include "std_msgs/msg/float32.hpp"

using namespace std::chrono_literals; 

#define PI 3.14159265358979323846

class XArmApiDemo : public rclcpp::Node
{
public:
  XArmApiDemo()
  : Node("xarm_api_demo")
  {
    hw_ns_ = this->declare_parameter<std::string>("hw_ns", "xarm");
    dof_ = this->declare_parameter<int>("dof", 7);

    //teenuste kliendid: liikumise lubamiseks, režiimi ja seisundi määramiseks ning poosi ja positsiooni edastamiseks
    motion_enable_client_ = this->create_client<xarm_msgs::srv::SetInt16ById>("/" + hw_ns_ + "/motion_enable");
    set_mode_client_ = this->create_client<xarm_msgs::srv::SetInt16>("/" + hw_ns_ + "/set_mode");
    set_state_client_ = this->create_client<xarm_msgs::srv::SetInt16>("/" + hw_ns_ + "/set_state");
    move_joint_client_ = this->create_client<xarm_msgs::srv::MoveJoint>("/" + hw_ns_ + "/set_servo_angle");
    move_cartesian_client_ = this->create_client<xarm_msgs::srv::MoveCartesian>("/" + hw_ns_ + "/set_position");

    //roboti asendid (rad): koduasend, asend kuubiku kohal enne haaramist, viskamise algusasend ja lõppasend
    joint_pos_1_ = { 0.0, 0.0,  0.0,  0.0,  0.0,  0.0,  0.0 };
    joint_pos_2_ = {30.2*PI/180.0, 22*PI/180.0, 2.1*PI/180.0, 40.1*PI/180.0, -2.6*PI/180.0, 18.1*PI/180.0, 34.7*PI/180.0 };
    joint_pos_3_ = { 0.0, 0.0, 0.0, 0.2, 0.0, 0.7, 0.0 };
    joint_pos_4_ = { 0.0, 0.0, 0.0, PI/2, 0.0, 0.0, 0.0 };

    //kuubiku haaramise positsioon (mm, mm, mm, rad, rad, rad)
    cart_pos_1_ = { 348.3, 224.1, 7, PI, 0.0, 0.0 };

    //haaratsi soovitud asendi kuulutaja
    gripper_publisher_ = this->create_publisher<std_msgs::msg::Float32>("gripper_moving_call", 10);
  }

  //roboti initsialiseerimine
  bool initialize_robot()
  {
    //logib infot
    RCLCPP_INFO(get_logger(), "xArm API teenuste ootamine...");

    //ootab, kuni kõik teenused on saadaval
    if (!wait_for_client(motion_enable_client_) ||
        !wait_for_client(set_mode_client_) ||
        !wait_for_client(set_state_client_) ||
        !wait_for_client(move_joint_client_) ||
        !wait_for_client(move_cartesian_client_))
    {
      return false;
    }

    //lubab liigendite liikumise
    auto enable_req = std::make_shared<xarm_msgs::srv::SetInt16ById::Request>();
    enable_req->id = 8;
    enable_req->data = 1;
    if (!call_simple(motion_enable_client_, enable_req, "motion_enable")) {
      return false;
    }

    //määrab töörežiimi väärtuseks "0"
    auto mode_req = std::make_shared<xarm_msgs::srv::SetInt16::Request>();
    mode_req->data = 0;
    if (!call_simple(set_mode_client_, mode_req, "set_mode")) {
      return false;
    }

    //määrab seisundiks "0"
    auto state_req = std::make_shared<xarm_msgs::srv::SetInt16::Request>();
    state_req->data = 0;
    if (!call_simple(set_state_client_, state_req, "set_state")) {
      return false;
    }

    RCLCPP_INFO(get_logger(), "Robot initsialiseeritud");
    return true;
  }

  //funktsioon haaratsi soovitud asendi kuulutamiseks
  void gripper_callback(float gripper_data){
    
    auto message = std_msgs::msg::Float32();
    message.data = gripper_data;
    
    RCLCPP_INFO(get_logger(), "Haaratsi asendi saatmine");
    gripper_publisher_->publish(message);
    };

  //programmi põhitsükkel
  bool run_demo()
  {
    //koduasend
    if (!move_joint(joint_pos_1_, 1, 10.0, true)) {
      return false;
    }

    //kuubiku kohale liikumine
    if (!move_joint(joint_pos_2_, 1, 20.0, true)) {
      return false;
    }
    
    //haaratsi avamine
    gripper_callback(0.2);
    sleep(2);

    //kuubiku haaramisasend
    if (!move_cartesian(cart_pos_1_, 120.0, 1000.0, true)) {
      return false;
    }

    //haaratsi sulgemine
    gripper_callback(0.57);
    sleep(2);

    //kuubiku tõstmine
    if (!move_joint(joint_pos_2_, 1, 20.0, true)) {
      return false;
    }

    //viske algasend (koduasend)
    if (!move_joint(joint_pos_3_, 1, 15.0, true)) {
      return false;
    }

    //viske lõppasend
    if (!move_joint(joint_pos_4_, 6, 100.0, false)) {
      return false;
    }
    
    rclcpp::sleep_for(std::chrono::milliseconds(450));
    gripper_callback(0.2); //haaratsi avamine
    
    //koduasend
     if (!move_joint(joint_pos_1_, 0.35, 10.0, false)) {
      return false;
    }

    RCLCPP_INFO(get_logger(), "Demo lõpp");
    return true;
  }

private:
  //funktsioon teenuste saadavuse ootamiseks
  template<typename ClientPtrT>
  bool wait_for_client(const ClientPtrT & client)
  {
    while (!client->wait_for_service(1s)) {   
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(get_logger(), "Katkestus teenuse ootamisel");
        return false;
      }
      RCLCPP_WARN(get_logger(), "Teenuse ootamine: %s", client->get_service_name());
    }
    return true;
  }

  template<typename ClientPtrT, typename RequestT>
  bool call_simple( //funktsioon kõigi tellimuste edastamiseks
    const ClientPtrT & client,
    const std::shared_ptr<RequestT> & req,
    const std::string & label)  
  {
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

  //funktsioon roboti poosi järgi juhtimiseks
  bool move_joint(const std::vector<double> & joints, double speed, double acc, bool wait)  
  { //kontrollib saadud asendite vastavust roboti liigendite arvuga
    if ((int)joints.size() != dof_) {
      RCLCPP_ERROR(get_logger(), "Liigendite arv ei kattu: eeldatud %d, anti %zu", dof_, joints.size());
      return false;
    }
    //määrab tellimuse tüübi ning määrab edastatud muutujad vastavusse selle andmeväljadega
    auto req = std::make_shared<xarm_msgs::srv::MoveJoint::Request>();
    req->angles = std::vector<float>(joints.begin(), joints.end());
    req->speed = speed;   
    req->acc = acc;      
    req->mvtime = 0.0;
    req->wait = wait;
    req->timeout = 0.0;
    req->radius = 20.0;

    RCLCPP_INFO(get_logger(), "Poosi liikumine: kiirus=%.3f kiirendus=%.3f ootamine=%d", speed, acc, wait);
    //tellimus edastatakse call_simple funktsiooniga
    return call_simple(move_joint_client_, req, "set_servo_angle");
  }

  //funktsioon roboti positsiooni järgi juhtimiseks
  bool move_cartesian(const std::vector<float> & pose, float speed, float acc, bool wait)
  {
    if (pose.size() != 6) {
      RCLCPP_ERROR(get_logger(), "Positsioon peab olema määratud 6 väärtusega.");
      return false;
    }
    //määrab tellimuse tüübi ning määrab edastatud muutujad vastavusse selle andmeväljadega
    auto req = std::make_shared<xarm_msgs::srv::MoveCartesian::Request>();
    req->pose = pose; 
    req->speed = speed; 
    req->acc = acc;     
    req->mvtime = 0.0;
    req->wait = wait;
    req->timeout = 0.0;

    RCLCPP_INFO(get_logger(), "Positsiooni liikumine: kiirus=%.1f kiirendus=%.1f ootamine=%d", speed, acc, wait);
    //tellimus edastatakse call_simple funktsiooniga
    return call_simple(move_cartesian_client_, req, "set_position");
  }

//kasutatud muutujate tüüpide defineerimine
private:
  std::string hw_ns_;
  int dof_;

  rclcpp::Client<xarm_msgs::srv::SetInt16ById>::SharedPtr motion_enable_client_;
  rclcpp::Client<xarm_msgs::srv::SetInt16>::SharedPtr set_mode_client_;
  rclcpp::Client<xarm_msgs::srv::SetInt16>::SharedPtr set_state_client_;
  rclcpp::Client<xarm_msgs::srv::MoveJoint>::SharedPtr move_joint_client_;
  rclcpp::Client<xarm_msgs::srv::MoveCartesian>::SharedPtr move_cartesian_client_;

  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
  rclcpp::Subscription<xarm_msgs::msg::RobotMsg>::SharedPtr robot_state_sub_;

  sensor_msgs::msg::JointState::SharedPtr last_joint_state_;
  xarm_msgs::msg::RobotMsg::SharedPtr last_robot_state_;

  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr gripper_publisher_;

  std::vector<double> joint_pos_1_;
  std::vector<double> joint_pos_2_;
  std::vector<double> joint_pos_3_;
  std::vector<double> joint_pos_4_;

  std::vector<float> cart_pos_1_;
};

//põhitsükkel erinevate programmi osade käivitamiseks
int main(int argc, char ** argv){
  rclcpp::init(argc, argv);
  auto node = std::make_shared<XArmApiDemo>();

  if (!node->initialize_robot()) {
    RCLCPP_ERROR(node->get_logger(), "Roboti initsialiseerimine ebaõnnestus");
    rclcpp::shutdown();
    return 1;
  }

  if (!node->run_demo()) {
    RCLCPP_ERROR(node->get_logger(), "Demo ebaõnnestus");
    rclcpp::shutdown();
    return 1;
  }

  rclcpp::shutdown();
  return 0;
}

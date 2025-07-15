#include "rclcpp/rclcpp.hpp"

// ROS2 messages
#include "sensor_msgs/msg/joy.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"  // Float32MultiArray 用

// SerialBridge for F446RE
#include "LinuxHardwareSerial.hpp"
#include "SerialBridge.hpp"

// Serial massages
#include "controller.hpp"
#include "feedback.hpp"


// SerialPort for Rpi4
#define SERIAL_PORT "/dev/serial/by-path/platform-fd500000.pcie-pci-0000:01:00.0-usb-0:1.4:1.2"

// ros2 runでエラー出たらとりあえずこれ
// sudo chmod 666 /dev/serial/by-path/platform-fd500000.pcie-pci-0000\:01\:00.0-usb-0\:1.4\:1.2

// SerialPort for ThinkPad
// #define SERIAL_PORT "/dev/serial/by-path/pci-0000:00:14.0-usb-0:5:1.2"

using namespace std::chrono_literals;
using std::placeholders::_1;

class SerialNode : public rclcpp::Node {
public:
  SerialNode(const char port[], speed_t baud_rate = B115200)
    : Node("serial_node"),
      serial_dev(new LinuxHardwareSerial(port, baud_rate)),
      serial(new SerialBridge(serial_dev, 1024)) {
    subscription_ = this->create_subscription<sensor_msgs::msg::Joy>("sp4/joy", 10,
                                                                     std::bind(&SerialNode::joy_callback, this, _1));
    publisher_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("fb_speed_topic", 10);  // Publisher を作成

    serial->add_frame(0, &con);
    serial->add_frame(1, &fb);
    // serial->add_frame(2, &gain);



    // タイマーの設定（シリアル通信）
    timer_ = this->create_wall_timer(10ms, std::bind(&SerialNode::serial_callback, this));
  }

private:
  void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) {
    lx = msg->axes[0] * -1;
    ly = msg->axes[1];
    rx = msg->axes[2] * -1;
    ry = msg->axes[3];
    l2 = msg->axes[4];
    r2 = msg->axes[5];

    cross = msg->buttons[0];
    circle = msg->buttons[1];
    square = msg->buttons[2];
    triangle = msg->buttons[3];

    l1 = msg->buttons[9];
    r1 = msg->buttons[10];

    down = msg->buttons[11];
    up = msg->buttons[12];
    left = msg->buttons[13];
    right = msg->buttons[14];

    // サイドアーム①
    con.data.motor1 = lx;

    // サイドアーム②
    con.data.motor2 = ly;

    // サイド昇降
    if (s3 == 1){
      con.data.motor3 = 0;
    } else {
      con.data.motor3 = rx * 0.3;
    }

    // フロント昇降
    if (s4 == 1){
      con.data.motor4 = 0;
    } else {
      con.data.motor4 = ry * 0.3;
    }

    // 吸盤
    con.data.motor5 = l2;

    // 予備
    con.data.motor6 = 0;

    serial->write(0);

    RCLCPP_INFO(this->get_logger(), "sent data... ");
  }


  void serial_callback() {

    int result = serial->update();
    if (result == 0)
    {
      RCLCPP_INFO(this->get_logger(), "Serial data updated successful");
    }
    else
    {
      RCLCPP_ERROR(this->get_logger(), "Serial data ERROR: %d", result);
    }

    s1 = fb.data.switches[0];
    s2 = fb.data.switches[1];
    s3 = fb.data.switches[2];
    s4 = fb.data.switches[3];
    s5 = fb.data.switches[4];
    s6 = fb.data.switches[5];

    printf("switch: %d %d %d %d %d %d\n", s1, s2, s3, s4, s5, s6);
  }

  void gain_setup() const {

  }

  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
  rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr publisher_;  // Publisher の宣言
  rclcpp::TimerBase::SharedPtr timer_;


  SerialDev *serial_dev;
  SerialBridge *serial;
  Controller con;
  Feedback fb;

  bool s1, s2, s3, s4, s5, s6;

  float lx, ly, rx, ry, l2, r2;
  bool cross, circle, triangle, square, l1, r1;
  bool up, down;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  auto unified_node = std::make_shared<SerialNode>(SERIAL_PORT);
  rclcpp::spin(unified_node);
  rclcpp::shutdown();
  return 0;
}

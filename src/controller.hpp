#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "Message.hpp"
#include "cstdint"

typedef struct ControllerType {

  float motor1;
  float motor2;
  float motor3;
  float motor4;
  float motor5;
  float motor6;

} controller_t;

//  create message
typedef sb::Message<controller_t> Controller;

#endif //CONTROLLER_HPP
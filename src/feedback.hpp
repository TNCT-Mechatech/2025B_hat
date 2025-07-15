#ifndef FEEDBACK_HPP
#define FEEDBACK_HPP

#include "Message.hpp"
#include "cstdint"

typedef struct FeedbackType {

  bool switches[6];
  

} feedback_t;

// create message
typedef sb::Message<feedback_t> Feedback;

#endif //FEEDBACK_HPP
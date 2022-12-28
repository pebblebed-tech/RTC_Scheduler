#pragma once
#include "esphome/core/component.h"
#include "esphome/components/select/select.h"



namespace esphome {
namespace rtc_scheduler {

class RTCSchedulerItemMode : public select::Select, public Component {
 protected:
  void control(const std::string &value) override { this->publish_state(value); }
};

}  // namespace rtc_scheduler
}  // namespace esphome
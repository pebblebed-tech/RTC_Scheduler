#pragma once
#include "rtc_scheduler.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace rtc_scheduler {
class RTCSchedulerTextSensor;         // Text sensor to display status to HA frontend
class RTCSchedulerTextSensor :  public text_sensor::TextSensor, public Component {
 public:
  
  void dump_config() override;
};
}
}

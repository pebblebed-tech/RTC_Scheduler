#pragma once
#include "esphome/core/component.h"
#include "esphome/components/select/select.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"
#include "rtc_text_sensor.h"
#include "rtc_scheduler.h"
#include "esphome/core/preferences.h"
#include <vector>
#include <string>


namespace esphome {
namespace rtc_scheduler {
class RTCSchedulerControllerSwitch;  // The scheduled switch 
class RTCSchedulerTextSensor;         // Text sensor to display status to HA frontend
class RTCSchedulerItemMode_Select : public select::Select, public Component {
  public:
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void setup() override;
  void dump_config() override;
  void configure_item(uint8_t item_slot_number,
                      RTCSchedulerControllerSwitch *item_sw,
                      switch_::Switch *item_sw_id,
                      RTCSchedulerTextSensor *item_status,
                      binary_sensor::BinarySensor* item_on_indicator
                      );


 protected:
  void control(const std::string &value) override; 
  std::string item_mode_state_ = "Not Configured Off";
  uint8_t item_slot_number_ = 0;
  RTCSchedulerControllerSwitch *item_sw_{nullptr};
  RTCSchedulerTextSensor *item_status_{nullptr};  
  binary_sensor::BinarySensor* item_on_indicator_ = nullptr; 
  switch_::Switch *item_sw_id_{nullptr};
  ESPPreferenceObject pref_;
};

}  // namespace rtc_scheduler
}  // namespace esphome
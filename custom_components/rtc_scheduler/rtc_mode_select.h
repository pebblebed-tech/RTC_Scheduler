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
enum ItemMode : size_t {
  ITEM_MODE_OFF = 0,
  ITEM_MODE_EARLY,
  ITEM_MODE_AUTO,
  ITEM_MODE_ON,
  ITEM_MODE_BOOST
};
enum ItemModeState : size_t {
  ITEM_STATE_INIT = 0,
  ITEM_STATE_OFF,
  ITEM_STATE_EARLY,
  ITEM_STATE_AUTO_OFF,
  ITEM_STATE_AUTO_ON,
  ITEM_STATE_ON,
  ITEM_STATE_BOOST
};
class RTCSchedulerItemMode_Select : public select::Select, public Component {
  public:
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void setup() override;
  void dump_config() override;
  void configure_item(uint8_t item_slot_number,
                      RTCSchedulerControllerSwitch *item_sw,
                      switch_::Switch *item_sw_id,
                      RTCSchedulerTextSensor *item_status,
                      RTCSchedulerTextSensor *item_next_event,
                      binary_sensor::BinarySensor* item_on_indicator
                      );
  void set_item_schedule_valid(bool schedule_valid);
  void set_controller_state(bool schedule_controller_state);
  bool get_controller_state();
  void set_scheduled_item_state(bool scheduled_item_state);
  bool get_scheduled_item_state();
  void set_next_scheduled_event(uint16_t next_schedule_event, bool next_schedule_state);
  uint8_t get_slot_number();
 protected:
  void adjustItemInternalState();
  void prepareItemSensorAndSwitches();
  void adjustItemSensorAndSwitches(bool newState,std::string &newValue);
  void adjustIndicatorState(bool newValue);
  void adjustSwitchState(bool newValue);
  void adjustStatusTextSensor(std::string &newValue);
  void adjustEventTextSensor(std::string &newValue);
  std::string convertEventTimeToStr(uint16_t event_time) const;
  void control(const std::string &value) override; 
  std::string item_mode_state_str_ = "Not Configured Off";
  // The scheduled item slot number used to get schedule from flash
  uint8_t item_slot_number_ = 0;
  // Optional switch to allow the scheduled item to iuse turn_on / turn_off 
  RTCSchedulerControllerSwitch *item_sw_{nullptr};
  // Optional Text sensor to display the status of the scheduled item to HA
  RTCSchedulerTextSensor *item_status_{nullptr};  
  // Optional Text sensor to display the next event for the scheduled item to HA
  RTCSchedulerTextSensor *item_next_event_{nullptr};  
  // Optional Binary Sensor to indicate to HA the state of the scheduled item
  binary_sensor::BinarySensor* item_on_indicator_ = nullptr; 
  // Optional switch ID EG GPIO ID so the scheduled item can control it
  switch_::Switch *item_sw_id_{nullptr};
  ESPPreferenceObject pref_;
  // Used by the scheduled item to maintian its controller state is either off or on
  bool schedule_controller_state_ = false;
  // Used by the scheduled item to maintian its switch state, is either off or on 
  bool schedule_controller_item_state_ = false;
  // Used to indicate whether the schedule for the item is valid or not
  bool schedule_valid_ = false;
  // Text string that holds the next schedule event 
  std::string next_schedule_event_str_ = "";
  // Event time in minutes (0-6 Days, 0-23 Hours, 0-59 minutes = Max 10079)
  uint16_t next_schedule_event_ = 10080; // Note any value above 10079 is invalid eg no event
  // Event next state EG off or on
  bool next_schedule_state_ = false;

  ItemMode item_mode_ = ITEM_MODE_OFF;
  ItemModeState item_mode_state_ = ITEM_STATE_INIT;
};

}  // namespace rtc_scheduler
}  // namespace esphome
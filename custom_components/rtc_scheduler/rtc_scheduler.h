#pragma once

#include "esphome/core/component.h"
#include "esphome/components/api/custom_api_device.h"
#include "../ext_eeprom_component/ext_eeprom_component.h"
#include "esphome/core/hal.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "rtc_text_sensor.h"


#include <vector>
#include <string>

namespace esphome {
namespace rtc_scheduler {

enum SchedulerMode : uint8_t {

  IDLE,      // Schedule Controller is off
  AUTO_RUN,    // Schedule Controller is running
  MANUAL_RUN,    // Switches are all under manual control
};
enum ScheduledSwState : uint8_t {

  MANUAL_OFF,  // Switch is manually off until mode is changed
  EARLY_OFF,  // Switch is manually off until next on event
  AUTO_OFF,    // Switch is scheduled off
  MANUAL_ON,   // Switch is manually on until mode is changed
  BOOST_ON,    // Switch is boosted on until next off event
  AUTO_ON      // Switch is scheduled on
};
enum ScheduledSwMode : uint8_t {

  AUTO_MODE,  // Switch is under scheduler control
  MANUAL_MODE,  // Switch is under manual control
  SETUP_MODE    // Schedule for switch is not available
  
};
struct struct_schedule_event
{
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  char action[6];
};
struct struct_schedule_storage
{
  uint16_t event_time;
  char action_str[6];
};

class RTCScheduler;                  // this component
class RTCSchedulerControllerSwitch;  // switches that appear in the front end; based on switch core
//class RTCSchedulerSwitch;            // switches representing any valve or pump; provides abstraction for latching valves
class RTCSchedulerTextSensor;         // Text sensor to display status to HA frontend

template<typename... Ts> class ShutdownAction;
template<typename... Ts> class StartAction;

/* class RTCSchedulerSwitch {
 public:
  RTCSchedulerSwitch();
  RTCSchedulerSwitch(switch_::Switch *sprinkler_switch);


  
  void loop();               // called as a part of loop(), used for latching valve pulses
  bool state();  // returns the switch's current state
  void set_off_switch(switch_::Switch *off_switch) { this->off_switch_ = off_switch; }
  void set_on_switch(switch_::Switch *on_switch) { this->on_switch_ = on_switch; }
  
  void sync_valve_state(
      bool latch_state);  // syncs internal state to switch; if latching valve, sets state to latch_state
  void turn_off();        // sets internal flag and actuates the switch
  void turn_on();         // sets internal flag and actuates the switch
  switch_::Switch *off_switch() { return this->off_switch_; }
  switch_::Switch *on_switch() { return this->on_switch_; }

 protected:
  bool state_{false};
  
//  switch_::Switch *off_switch_{nullptr};  // only used for latching valves
//  switch_::Switch *on_switch_{nullptr};   // used for both latching and non-latching valves

}; */

/* class RTCSchedulerTextSensor :  public text_sensor::TextSensor, public Component {
 public:
  
  void dump_config() override;
}; */
// *********************************************************************************************
class RTCSchedulerControllerSwitch : public switch_::Switch, public Component {
 public:
  RTCSchedulerControllerSwitch();

  void setup() override;
  void dump_config() override;

  void set_state_lambda(std::function<optional<bool>()> &&f);
  void set_restore_state(bool restore_state);
  Trigger<> *get_turn_on_trigger() const;
  Trigger<> *get_turn_off_trigger() const;
  void set_optimistic(bool optimistic);
  void set_assumed_state(bool assumed_state);
  void loop() override;

  float get_setup_priority() const override;
  

 
 protected:
  bool assumed_state() override;

  void write_state(bool state) override;

  optional<std::function<optional<bool>()>> f_;
  bool optimistic_{true};
  bool assumed_state_{false};
  Trigger<> *turn_on_trigger_;
  Trigger<> *turn_off_trigger_;
  Trigger<> *prev_trigger_{nullptr};
  bool restore_state_{false};

}; 

class RTCScheduler : public Component, public api::CustomAPIDevice, public EntityBase {
 public:
  RTCScheduler();
  RTCScheduler(const std::string &name);  
  void setup() override;
  void loop() override;
  void dump_config() override;
  void test();
  
  void on_schedule_recieved(int schedule_device_id, int event_count,  std::vector<int> days ,std::vector<int> hours ,std::vector<int> minutes, std::vector<std::string> actions);
  void send_log_message_to_HA(String level, String logMessage, String sender);
  void on_schedule_erase_recieved(int schedule_device_id);
  void on_erase_all_schedules_recieved();
  void set_Storage_Offset(uint16_t storage_offset) { this->storage_offset_ = storage_offset; }
  void set_storage(ext_eeprom_component::ExtEepromComponent *storage) { this->storage_ = storage; }
  void set_Switch_Count(uint8_t switch_count) { this->switch_count_ = switch_count; }
  void set_Events_Per_Switch(uint16_t max_switch_events) { this->max_switch_events_ = max_switch_events; }
  float get_setup_priority() const override;
  /// add another controller to the controller so it can check slots
  void add_controller(RTCScheduler *other_controller);
  void set_controller_main_switch(RTCSchedulerControllerSwitch *controller_switch);
  void resume_or_start_schedule_controller();
  void shutdown_schedule_controller();
  void set_main_switch_status(RTCSchedulerTextSensor *controller_Status);
  protected:
        ext_eeprom_component::ExtEepromComponent *storage_;
        uint16_t storage_offset_;
        uint8_t switch_count_;
        uint16_t max_switch_events_;
        const int event_size =9;  // the event is 9 bytes (3 bytes for time and 6 for  action string)
 /// Other Controller instances we should be aware of (used to check if slots are conflicting)
  std::vector<RTCScheduler *> other_controllers_;
  RTCSchedulerControllerSwitch *controller_sw_{nullptr};
  RTCSchedulerTextSensor *controllerStatus_{nullptr};
  std::unique_ptr<ShutdownAction<>> scheduler_shutdown_action_;
  std::unique_ptr<StartAction<>> scheduler_start_action_;
  std::unique_ptr<Automation<>> scheduler_turn_off_automation_;
  std::unique_ptr<Automation<>> scheduler_turn_on_automation_;
};


}  // namespace scheduler_component
}  // namespace esphome
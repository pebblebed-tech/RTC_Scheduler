#pragma once

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/api/custom_api_device.h"
#include "../external_eeprom/external_eeprom.h"
#include "esphome/core/hal.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/select/select.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "rtc_text_sensor.h"
#include "rtc_mode_select.h"
#include "rtc_scheduler_hub.h"

#include <vector>
#include <string>

namespace esphome {
namespace rtc_scheduler {
/* 
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
*/
struct struct_schedule_event
{
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  bool action;
};
/*
struct struct_schedule_storage
{
  uint16_t event_time;
  char action_str[6];
}; */
static const uint16_t SCHEDULER_VALID_WORD_1 = 0xAA55;
//static const uint16_t SCHEDULER_VALID_WORD_1 = 0xAA66;
static const uint16_t SCHEDULER_VALID_WORD_2 = 0xF0F0;

static const uint16_t SLOT_INVALID_WORD_1 = 0x55AA;
//static const uint16_t SLOT_INVALID_WORD_1 = 0x66AA;
static const uint16_t SLOT_VALID_WORD_1 = 0x5A5A;


class RTCScheduler;                  // this component
class RTCSchedulerControllerSwitch;  // Main switch for the controller
class RTCSchedulerTextSensor;         // Text sensor to display status to HA frontend
class RTCSchedulerItemMode_Select;    // Select that sets the mode of the scheduled item
class RTCSchedulerHub;                // The master component
template<typename... Ts> class ShutdownAction;
template<typename... Ts> class StartAction;

//*****************************************************************************
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

class RTCScheduler : public Component, public api::CustomAPIDevice  {
  public:
    //RTCScheduler();
    //RTCScheduler(const std::string &name);  
    void setup() override;
    void loop() override;
    void dump_config() override;
    void test();
    void set_name(const std::string &name) { this->name_ = name; }
    void on_schedule_recieved(int schedule_slot_id,  std::vector<int> days ,std::vector<int> hours ,std::vector<int> minutes, std::vector<std::string> &actions);
    void on_text_schedule_recieved(int schedule_slot_id, std::string &events); 
    void send_log_message_to_HA(String level, String logMessage, String sender);
    void on_schedule_erase_recieved(int schedule_slot_id);
    void on_erase_all_schedules_recieved();
    void set_storage_offset(uint16_t storage_offset) { this->storage_offset_ = storage_offset; }
    void set_storage(external_eeprom::ExtEepromComponent *storage) { this->storage_ = storage; }
    void set_scheduled_items_count_(uint8_t items_count) { this->scheduled_items_count_ = items_count; }
    void set_events_per_switch(uint16_t max_switch_events) { this->max_switch_events_ = max_switch_events; }
    float get_setup_priority() const override;
    /// add another controller to the controller so it can check slots
    void add_controller(RTCScheduler *other_controller);
    void set_controller_main_switch(RTCSchedulerControllerSwitch *controller_switch);
    void resume_or_start_schedule_controller();
    void shutdown_schedule_controller();
    void set_main_switch_status(RTCSchedulerTextSensor *controller_Status);
    void set_ind(binary_sensor::BinarySensor *s) { ctl_on_sensor_ = s; }
    void add_scheduled_item(uint8_t item_slot_number,
                        RTCSchedulerControllerSwitch *item_sw,
                        switch_::Switch *item_sw_id,
                        RTCSchedulerTextSensor *item_status,
                        RTCSchedulerTextSensor *item_next_event,
                        RTCSchedulerItemMode_Select *item_mode_select,
                        binary_sensor::BinarySensor* item_on_indicator
                        );
    void set_parent(RTCSchedulerHub *parent) { this->parent_ = parent; };
    
    void set_slot_valid(uint8_t item_slot_number, bool valid);
    void set_slot_sw_state(uint8_t item_slot_number, bool sw_state);
    bool get_storage_status();
    //std::vector<uint16_t> split_and_convert(std::string s);
    //std::vector<Data> splitCsvData(const std::string& csvData);

  protected:
    external_eeprom::ExtEepromComponent *storage_;
    uint16_t storage_offset_;
    bool storage_configured = false;
    uint8_t scheduled_items_count_ = 0;      // Number active scheduled items
    uint16_t slot_size_ = 0;            // This is num of bytes for a slot
    uint16_t max_switch_events_;
    bool storage_valid_ = false;
    bool check_the_cksm(uint8_t slot);
    uint16_t calculate_slot_cksm(uint8_t slot);
    uint16_t get_slot_starting_address(uint8_t slot);
    void configure_storage();
    std::string name_;
    RTCSchedulerItemMode_Select* get_scheduled_item_from_slot(uint8_t slot);
    // Other Controller instances we should be aware of (used to check if slots are conflicting)
    std::vector<RTCScheduler *> other_controllers_;
    // List of scheduled item components
    std::vector<RTCSchedulerItemMode_Select*> scheduled_items_;
    // Vector to hold event times before being stored
    std::vector<struct_schedule_event> events_for_storage_;
  
    std::string controller_mode_state_;
    RTCSchedulerControllerSwitch *controller_sw_{nullptr};
    RTCSchedulerTextSensor *controllerStatus_{nullptr};
    RTCSchedulerItemMode_Select *item_mode_select_ = nullptr;  
    binary_sensor::BinarySensor* ctl_on_sensor_ = {nullptr};           
    RTCSchedulerHub *parent_={nullptr};
    std::unique_ptr<ShutdownAction<>> scheduler_shutdown_action_;
    std::unique_ptr<StartAction<>> scheduler_start_action_;
    std::unique_ptr<Automation<>> scheduler_turn_off_automation_;
    std::unique_ptr<Automation<>> scheduler_turn_on_automation_;
};


}  // namespace scheduler_component
}  // namespace esphome
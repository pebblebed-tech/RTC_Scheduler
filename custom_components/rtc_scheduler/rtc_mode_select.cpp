#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "rtc_mode_select.h"
namespace esphome {
namespace rtc_scheduler {
static const char *TAG = "rtc_scheduler_scheduled_item ";
 void RTCSchedulerItemMode_Select::dump_config()
{
  LOG_SELECT("", "RTC Schedule Item", this);
 
} 

void RTCSchedulerItemMode_Select::setup()
{
  std::string value;
  size_t index;
  this->pref_ = global_preferences->make_preference<size_t>(this->get_object_id_hash());
  if (!this->pref_.load(&index)) {
    value = "Manual Off";
    ESP_LOGD(TAG, "State from initial (could not load): %s", value.c_str());
  } else {
    value = this->traits.get_options().at(index);
    ESP_LOGD(TAG, "State from restore: %s", value.c_str());
  }
  this->publish_state(value);  
}
// Setup the scheduled item with its various components
void RTCSchedulerItemMode_Select::configure_item(uint8_t item_slot_number, RTCSchedulerControllerSwitch *item_sw, switch_::Switch *item_sw_id, RTCSchedulerTextSensor *item_status, RTCSchedulerTextSensor *item_next_event,binary_sensor::BinarySensor *item_on_indicator)
{
  this->item_slot_number_ = item_slot_number;
  this->item_sw_ = item_sw;
  this->item_sw_id_ = item_sw_id;
  if (this->item_sw_id_ != nullptr){
    this->item_sw_id_->set_internal(true);
  }
  if (this->item_sw_ != nullptr){
    this->item_sw_->set_internal(true);
  }
  this->item_status_ = item_status;
  if (this->item_status_ != nullptr){
    this->item_status_->publish_state("Initialising");
  }
  
  this->item_next_event_ = item_next_event;
  if (this->item_next_event_ != nullptr){
    this->item_next_event_->publish_state("None");
  }

  this->item_on_indicator_ = item_on_indicator;
  if (this->item_on_indicator_ != nullptr){
    this->item_on_indicator_->publish_state(false);
  }
  
}
// Tell the scheduled item that its controller is either off or on
void RTCSchedulerItemMode_Select::set_controller_state(bool schedule_controller_state)
{
  this->schedule_controller_state_= schedule_controller_state;
}

void RTCSchedulerItemMode_Select::get_controller_state(bool schedule_controller_state)
{
   schedule_controller_state = this->schedule_controller_state_;
}
// Tell the scheduled item that it should turn  off or on by schedule
void RTCSchedulerItemMode_Select::set_scheduled_item_state(bool scheduled_item_state)
{
  this->schedule_controller_item_state_ = scheduled_item_state;
}

void RTCSchedulerItemMode_Select::get_scheduled_item_state(bool scheduled_item_state)
{
  scheduled_item_state = this->schedule_controller_item_state_;
}
 void RTCSchedulerItemMode_Select::set_item_schedule_valid(bool schedule_valid){
    schedule_valid_ = schedule_valid;
 }
// Depending on the item mode driven by the HA select and the current schedule controller EG (On or Off) Adjust the rest of the internal states 
void RTCSchedulerItemMode_Select::adjustItemInternalState()
{
// First lets deal with the corner case of a invalid schedule / start-up
  if (not schedule_valid_){
    item_mode_state_ = ITEM_STATE_INIT;
    item_mode_ = ITEM_MODE_OFF;

  }
  else{
    switch (this->item_mode_)
    {
    case ItemMode::ITEM_MODE_OFF:
      item_mode_state_ = ITEM_STATE_OFF;
      break;
    case ItemMode::ITEM_MODE_ON:
      item_mode_state_ = ITEM_STATE_ON;
      break;
    case ItemMode::ITEM_MODE_EARLY:
      item_mode_state_ = ITEM_STATE_EARLY;
      break;
    case ItemMode::ITEM_MODE_BOOST:
      item_mode_state_ = ITEM_STATE_BOOST;
      break;
    case ItemMode::ITEM_MODE_AUTO:
      if (schedule_controller_item_state_)
        item_mode_state_ = ITEM_STATE_AUTO_ON;
      else
        item_mode_state_ = ITEM_STATE_AUTO_OFF;
      break;
    }
  }

  this->prepareItemSensorAndSwitches();
}
// Driven by the internal mode state update the switches, indicator and status text
void RTCSchedulerItemMode_Select::prepareItemSensorAndSwitches()
{
  switch (this->item_mode_state_)
  {
  case ItemModeState::ITEM_STATE_INIT
    item_mode_state_str_ = "Not configured"
    break;
  
  default:
    break;
  }

}
//TODO Sort out states and make sure strings are handle right
void RTCSchedulerItemMode_Select::adjustItemSensorAndSwitches(bool newState,std::string newValue )
{
  adjustIndicatorState(newState);
  adjustSwitchState(newState);
  adjustStatusTextSensor(newValue);
}

void RTCSchedulerItemMode_Select::adjustIndicatorState(bool newValue)
{
  if (this->item_on_indicator_!= nullptr)
      this->item_on_indicator_->publish_state(newValue);
}
void RTCSchedulerItemMode_Select::adjustSwitchState(bool newValue)
{
  if (newValue)
  {
    if (this->item_sw_!= nullptr)
      this->item_sw_->turn_on();
    if (this->item_sw_id_!= nullptr)
      this->item_sw_id_->turn_on();
  }
  else{
    if (this->item_sw_!= nullptr)
      this->item_sw_->turn_off();
    if (this->item_sw_id_!= nullptr)
      this->item_sw_id_->turn_off();
  }
}
void RTCSchedulerItemMode_Select::adjustStatusTextSensor(std::string newValue)
{
  if (this->item_status_ != nullptr){
    this->item_status_->publish_state(newValue.c_Str());
}
// Allow the schedule item to display its next event if the text sensor is configured
void RTCSchedulerItemMode_Select::set_next_scheduled_event(std::string next_schedule_event)
{
   this->next_schedule_event_ =  next_schedule_event; 
}

void RTCSchedulerItemMode_Select::control(const std::string &value)
{
  ESP_LOGD(TAG, "Setting slot %d item mode %s",this->item_slot_number_,value.c_str());
  this->publish_state(value);
  // save the state
  auto options = this->traits.get_options();
  size_t index = std::find(options.begin(), options.end(), value) - options.begin();
  this->pref_.save(&index);
  ESP_LOGD(TAG, "Saving slot %d index %d",index);
  this->item_mode_ = static_cast<ItemMode>(index);
}
} // namespace rtc_scheduler
}  // namespace esphome
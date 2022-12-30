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

void RTCSchedulerItemMode_Select::configure_item(uint8_t item_slot_number, RTCSchedulerControllerSwitch *item_sw, switch_::Switch *item_sw_id, RTCSchedulerTextSensor *item_status, binary_sensor::BinarySensor *item_on_indicator)
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

  this->item_on_indicator_ = item_on_indicator;
  if (this->item_on_indicator_ != nullptr){
    this->item_on_indicator_->publish_state(false);
  }
  
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
  this->item_mode_state_ =value;
  this->item_status_->publish_state(value.c_str());
  if (this->item_mode_state_ == "Manual On"){
    this->item_on_indicator_->publish_state(true);
    this->item_sw_->turn_on();
    this->item_sw_id_->turn_on();

  }
  else{
    this->item_on_indicator_->publish_state(false);
    this->item_sw_->turn_off();
    this->item_sw_id_->turn_off();
  }
}
} // namespace rtc_scheduler
}  // namespace esphome
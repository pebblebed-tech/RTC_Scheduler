#include "automation.h"
#include "rtc_scheduler.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include <utility>
#include <string>

namespace esphome {
namespace rtc_scheduler {

static const char *TAG = "rtc_scheduler";
// TODO Add timing code
RTCScheduler::RTCScheduler() {}
RTCScheduler::RTCScheduler(const std::string &name) : EntityBase(name) {}

void RTCScheduler::setup() {

}

void RTCScheduler::loop() {
 if (!storage_configured)
 {
  this->configure_storage();
 }
    

// Check schedule events against current time
// If event time is in past then set switch states per schedule unless override is active
// Update next schedule next event per switch
}

void RTCScheduler::dump_config(){
    ESP_LOGCONFIG(TAG, "Scheduler component");
    ESP_LOGCONFIG(TAG, "RTC Scheduler Controller -- %s", this->name_.c_str());
     std::string service_sched_name;
     service_sched_name =  this->name_;
 // remove the spaces
 for (size_t i = 0; i < service_sched_name.size(); ++i) {
    if (service_sched_name[i] == ' ') {
        service_sched_name.replace(i, 1, "_");
    }
}
ESP_LOGCONFIG(TAG, "RTC Scheduler Controller name -- %s", service_sched_name.c_str());
    // TODO dump config for EEPROM and Switches

    
}
void RTCScheduler::configure_storage(){
  storage_configured = true;
  storage_valid_ = false;
  // Calc the scheduler slot size
  this->slot_size_ = (this->max_switch_events_ * 2) + 4; // convert numbers events to bytes and add 4 for config words
  //  lets check eeprom is valid
  if (storage_!=nullptr)
    {
      uint16_t data1;
      uint16_t data2;
      uint16_t storage_address;
      this->storage_->get(this->storage_offset_, data1);
      this->storage_->get(storage_offset_+2, data2);
      if (( data1 == SCHEDULER_VALID_WORD_1 ) and ( data2 == SCHEDULER_VALID_WORD_2 ))
        {
          storage_valid_ = true;
           ESP_LOGD(TAG, "Scheduler good - looking for valid slots");
          for (uint16_t i = 0; i < this->scheduled_items_.size(); i++)  // step through the items in the slot
            {
               // Check if slot is valid, if not move on
                this->storage_->get(this->get_slot_starting_address(i),data1); 
                if (data1 == SLOT_VALID_WORD_1)
                {
                     // Check if checksum is correct
                    if (this->check_the_cksm(i))
                    {
                      // Now checks have passed set item to valid
                      this->set_slot_valid(i,true);
                       ESP_LOGD(TAG, "Scheduler - Slot %d valid", i+1);
                       //TODO Need set sw state of the item depending on mode and previous state from schedule
                       
                    }
                    else
                    {
                        // Checksum failed so set item to invalid
                      this->set_slot_valid(i,false);
                      ESP_LOGD(TAG, "Scheduler - Slot %d checksum failed", i+1);
                    }
                }
                else
                {
                   ESP_LOGD(TAG, "Scheduler - Slot %d not valid", i+1);
                }
            }

        }
      else
        {
          // not configured so configure it now
          data1=SCHEDULER_VALID_WORD_1;
          data2=SCHEDULER_VALID_WORD_2;
          this->storage_->put(this->storage_offset_,data1);
          this->storage_->put(this->storage_offset_+2,data2);
          storage_valid_ = true;
          data1 = SLOT_INVALID_WORD_1;
          // by definition all slots in this scheduler are invalid
          // can happen if the number of slots of previous scheduler changes
          // Now setup the slots for this scheuler (This means setting them not valid)
          // first check the number of slots and max events for this scheduler are valid for safety
          if ((this->max_switch_events_>=1) and (this->scheduled_items_count_>=1))
            {
              for (uint16_t i = 0; i < this->scheduled_items_.size(); i++)  // step through the sw items
               { 
                this->storage_->put(this->get_slot_starting_address(i),data1);
                //  set item to invalid
                this->set_slot_valid(i,false);
               } 
              // TODO Send message to ha to tell the user to programme the schedules
              this->parent_->send_notification_to_ha("Scheduler Ready", "Need to send schedule to"+this->name_+"for scheduler to opperate","432" );
              
               ESP_LOGD(TAG, "Scheduler storage ready - HA user needs to send schedules");  
            }
          else
            {
              // Should never get here as config validation should cover this
              storage_valid_ = false;
              ESP_LOGD(TAG, "Config not correct - scheduler will not opperate");
            }
        }
    }
  else
    {
        storage_valid_ = false;
        ESP_LOGD(TAG, "Storage not detected - scheduler will not opperate");
        // TODO raise error with HA
        this->parent_->send_notification_to_ha("Scheduler Error", "Storage not detected","433" );
    }
}
void RTCScheduler:: test(){
                ESP_LOGD(TAG, "Mem size in bytes: %d",this->storage_->length());
                int myValue2 = -366;
                this->storage_->put(10, myValue2); //(location, data)
                int myRead2;
                this->storage_->get(10, myRead2); //location to read, thing to put data into
                ESP_LOGD(TAG, "I read: %d",myRead2 );
}


void RTCScheduler::on_text_schedule_recieved(int schedule_slot_id, std::string &events) {
    ESP_LOGD(TAG, "%s Text Schedule Slot %d   recieved %s",this->name_.c_str(), schedule_slot_id, events.c_str());
    this->parent_->send_notification_to_ha("Text Rxed","Have rx a text schedule","103");
    
    //TODO convert to internal format and store it

}

void RTCScheduler::on_schedule_recieved(int schedule_slot_id,  std::vector<int> days ,std::vector<int> hours ,std::vector<int> minutes, std::vector<std::string> &actions) {
    ESP_LOGD(TAG, "%s Schedule Slot %d   recieved",this->name_.c_str(), schedule_slot_id);
    ESP_LOGD(TAG, "Entries Count - Day:%d, Hours: %d Mins:%d, Actions: %d",days.size(),hours.size(), minutes.size(), actions.size() );
    this->parent_->send_log_message_to_ha("error","The test message from Controller","ESPHome: boiler_controller");
    //TODO convert to internal format and store it
    // Verify and write data to eeprom
   
    // Setup next schedule next event per switch
}

void  RTCScheduler::on_schedule_erase_recieved(int schedule_slot_id){
    ESP_LOGD(TAG, "%s Schedule Slot %d  erase recieved",this->name_.c_str() ,schedule_slot_id);
    // TODO Mark slot as inactive and clear data
}
void  RTCScheduler::on_erase_all_schedules_recieved(){
        ESP_LOGD(TAG, "%s Erase all schedules recieved",this->name_.c_str());
        // TODO Mark all slots as inactive and clear data and disable schedule loop
}
void RTCScheduler::add_controller(RTCScheduler *other_controller)
{
    this->other_controllers_.push_back(other_controller);
}

void RTCScheduler::set_controller_main_switch(RTCSchedulerControllerSwitch *controller_switch)
{
  this->controller_sw_ = controller_switch;
  this->controller_sw_->set_restore_state(true);
  this->scheduler_turn_off_automation_ = make_unique<Automation<>>(controller_switch->get_turn_off_trigger());
  this->scheduler_shutdown_action_ = make_unique<rtc_scheduler::ShutdownAction<>>(this);
  this->scheduler_turn_off_automation_->add_actions({scheduler_shutdown_action_.get()});

  this->scheduler_turn_on_automation_ = make_unique<Automation<>>(controller_switch->get_turn_on_trigger());
  this->scheduler_start_action_ = make_unique<rtc_scheduler::StartAction<>>(this);
  this->scheduler_turn_on_automation_->add_actions({scheduler_start_action_.get()});

}

float RTCScheduler::get_setup_priority() const { return setup_priority::DATA; }

void RTCScheduler::resume_or_start_schedule_controller()
{
  //TODO write scheduler start up code
    ESP_LOGD(TAG, "Startup called");
    if (this->controllerStatus_ != nullptr) {
      this->controllerStatus_->publish_state("Controller On");
      this->ctl_on_sensor_->publish_state(true);

    }
  
}
void RTCScheduler::shutdown_schedule_controller()
{
  //TODO write scheduler shutdown code
  ESP_LOGD(TAG, "Shutdown called");
    if (this->controllerStatus_ != nullptr) {
      this->controllerStatus_->publish_state("Controller Off");
      this->ctl_on_sensor_->publish_state(false);

    }
}
void RTCScheduler::set_main_switch_status(RTCSchedulerTextSensor *controller_Status)
{
  controllerStatus_ = controller_Status;
  if (this->controllerStatus_ != nullptr) {
    controllerStatus_->publish_state("Initialising");
  }
}

void RTCScheduler::add_scheduled_item(uint8_t item_slot_number, RTCSchedulerControllerSwitch *item_sw, switch_::Switch *item_sw_id,RTCSchedulerTextSensor *item_status,RTCSchedulerTextSensor *item_next_event, RTCSchedulerItemMode_Select *item_mode_select, binary_sensor::BinarySensor *item_on_indicator)
{
   this->item_mode_select_ = item_mode_select;
   //TODO need to update the validity of the slot before calling the configure
   this->scheduled_items_.push_back(this->item_mode_select_); // Add to the list of scheduled items
   // Configure the new scheduled item
   this->scheduled_items_count_++;  // increase the number of schedule items by 1
   this->item_mode_select_->configure_item(item_slot_number, item_sw,item_sw_id,item_status, item_next_event,  item_on_indicator);
}
RTCSchedulerItemMode_Select* RTCScheduler::get_scheduled_item_from_slot(uint8_t slot)
{
  for (int i = 0; i < this->scheduled_items_.size(); i++) {
    if (this->scheduled_items_[i]->get_slot_number() == slot){
      ESP_LOGD(TAG, "Found slot");
      return  &(*this->scheduled_items_[i]);

    }
  }
  return nullptr;
}
uint16_t RTCScheduler::get_slot_starting_address(uint8_t slot)
{
  // calc the address of the start of the slot (offset + 4 bytes for scheduler valid) plus (slot size in bytes  * the slot)
  return ((this->storage_offset_ + 4) + (this->slot_size_  * slot));
}
bool RTCScheduler::check_the_cksm(uint8_t slot)
{
  uint16_t stored_checksum;
  // read what is stored
  this->storage_->get(get_slot_starting_address(slot)+2,stored_checksum); 
  if (this->calculate_slot_cksm(slot) == stored_checksum)
  {
    return true;
  }
  return false;
}
uint16_t RTCScheduler::calculate_slot_cksm(uint8_t slot)
{
  uint16_t data_addr = get_slot_starting_address(slot)+4;  
  uint16_t len = this->slot_size_ * 2; // 16bit word per event so * 2
  uint16_t crc = 0xFFFF;
  uint8_t data;
  while (len--) {
    // read the data from storage
    this->storage_->get(data_addr, data);
    crc ^= data;
    data_addr++;
    for (uint8_t i = 0; i < 8; i++) {
      if ((crc & 0x01) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

void RTCScheduler::set_slot_valid(uint8_t item_slot_number, bool valid)
{
  RTCSchedulerItemMode_Select* sched_item = get_scheduled_item_from_slot(item_slot_number);
  if(sched_item != nullptr){
    sched_item->set_item_schedule_valid(valid);
    ESP_LOGD(TAG, "setting slot %d", item_slot_number);
  }
}
void RTCScheduler::set_slot_sw_state(uint8_t item_slot_number, bool sw_state)
{
  RTCSchedulerItemMode_Select* sched_item = get_scheduled_item_from_slot(item_slot_number);
  if(sched_item != nullptr)
    sched_item->set_scheduled_item_state(sw_state);
}

bool RTCScheduler::get_storage_status()
{
  return storage_valid_;
  
}
//******************************************************************************************

RTCSchedulerControllerSwitch::RTCSchedulerControllerSwitch()
    : turn_on_trigger_(new Trigger<>()), turn_off_trigger_(new Trigger<>())
{
   
}

void RTCSchedulerControllerSwitch::setup()
{
   
  if (!this->restore_state_)
    return;

  auto restored = this->get_initial_state();
  if (!restored.has_value())
    return;

  ESP_LOGD(TAG, "  Restored state %s", ONOFF(*restored));
  if (*restored) {
    this->turn_on();
  } else {
    this->turn_off();
  }
}

void RTCSchedulerControllerSwitch::dump_config()
{
    LOG_SWITCH("", "RTCSchedulerController Switch", this);
  ESP_LOGCONFIG(TAG, "  Restore State: %s", YESNO(this->restore_state_));
  ESP_LOGCONFIG(TAG, "  Optimistic: %s", YESNO(this->optimistic_));

}

void RTCSchedulerControllerSwitch::set_state_lambda(std::function<optional<bool>()> &&f) { this->f_ = f;}


void RTCSchedulerControllerSwitch::set_restore_state(bool restore_state)
{
    this->restore_state_ = restore_state;
}

Trigger<> *RTCSchedulerControllerSwitch::get_turn_on_trigger() const { return this->turn_on_trigger_; }

Trigger<> *RTCSchedulerControllerSwitch::get_turn_off_trigger()  const { return this->turn_off_trigger_; }

void RTCSchedulerControllerSwitch::set_optimistic(bool optimistic)
{
    this->optimistic_ = optimistic;
}

void RTCSchedulerControllerSwitch::set_assumed_state(bool assumed_state)
{
    this->assumed_state_ = assumed_state; 
}

void RTCSchedulerControllerSwitch::loop()
{
      if (!this->f_.has_value())
    return;
  auto s = (*this->f_)();
  if (!s.has_value())
    return;

  this->publish_state(*s);
}

float RTCSchedulerControllerSwitch::get_setup_priority() const { return setup_priority::HARDWARE; }

bool RTCSchedulerControllerSwitch::assumed_state()
{
    return this->assumed_state_;
}

void RTCSchedulerControllerSwitch::write_state(bool state)
{
      if (this->prev_trigger_ != nullptr) {
    this->prev_trigger_->stop_action();
  }

  if (state) {
      this->prev_trigger_ = this->turn_on_trigger_;
      this->turn_on_trigger_->trigger();
    } 
  else {
      this->prev_trigger_ = this->turn_off_trigger_;
      this->turn_off_trigger_->trigger();
  }
  if (this->optimistic_)
    this->publish_state(state);
}

// TODO add switch setting code
//TODO add switch monitoring code
//TODO Add service to accept string schedule per slot from text boxes
RTCSchedulerControllerSwitch *controller_sw_{nullptr};

/* void RTCSchedulerTextSensor::dump_config()
{
  LOG_TEXT_SENSOR("  ", "TextSensor ", this);
} */
} // namespace rtc_scheduler
}  // namespace esphome
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

RTCScheduler::RTCScheduler() {}
RTCScheduler::RTCScheduler(const std::string &name) : EntityBase(name) {}

void RTCScheduler::setup() {
 
 //std::replace(service_sched_name.begin(), service_sched_name.end(), ' ', '_');
 //const std::string foo = std::string("b")+this->name_.c_str();
 
 //const std::string foo = "pump_scheduler";
 //service_name = "_send_schedula";
 //const std::string on_sched_name  ="bHeater Scheduler" + std::string("_send_schedule");

 //const std::string foo;
 
/*     const std::string foo = "heater_scheduler_send_schedulb";
  
   const std::string foobar = "pump_scheduler_send_schedulb";
 
const std::string foobart = "Heater_Scheduler"; 
  //on_sched_name = service_sched_name+service_name;
  if (service_sched_name == foobart )
    register_service(&RTCScheduler::on_schedule_recieved, foo,
                   {"schedule_slot_id",  "days", "hours","minutes","actions"});
     else
    register_service(&RTCScheduler::on_schedule_recievedb, foobar,
                   {"schedule_slot_id",  "days", "hours","minutes","actions"}); 
  service_name = "_erase_schedule";
  service_name = service_sched_name+service_name;
                  
  register_service(&RTCScheduler::on_schedule_erase_recieved, service_name,{"schedule_slot_id"});
  service_name = "_erase_all_schedules";
  service_name = service_sched_name+service_name;
  register_service(&RTCScheduler::on_erase_all_schedules_recieved, service_name); 

  service_name = "_send_text_schedule";
  service_name = service_sched_name+service_name;
                  
  register_service(&RTCScheduler::on_text_schedule_recieved, service_name,{"schedule_slot_id", "events"}); */

// TODO Need to validate each slot and keep a list of slot validity
       // Check schedule data in eeprom is valid
        // Setup next schedule next event per switch

}
void RTCScheduler::loop() {
    

// Check schedulke events against current time
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
void RTCScheduler:: test(){
                ESP_LOGD(TAG, "Mem size in bytes: %d",this->storage_->length());
                int myValue2 = -366;
                this->storage_->put(10, myValue2); //(location, data)
                int myRead2;
                this->storage_->get(10, myRead2); //location to read, thing to put data into
                ESP_LOGD(TAG, "I read: %d",myRead2 );
}

void RTCScheduler:: send_log_message_to_HA(String level, String logMessage, String sender)
{
       call_homeassistant_service("system_log.write", {
                                                    {"message", logMessage.c_str()},
                                                    {"level",   level.c_str()},
                                                    {"logger", sender.c_str()},
                                                });
        call_homeassistant_service("persistent_notification.create", {
                                                    {"title", logMessage.c_str()},
                                                    {"message",   "This is a test notification"},
                                                    {"notification_id", "103"},
                                                }); 
        fire_homeassistant_event("esphome.something_happened", {
      {"my_value", "500"},
    });
}
void RTCScheduler::on_text_schedule_recieved(int schedule_slot_id, std::string &events) {
    ESP_LOGD(TAG, "%s Text Schedule Slot %d   recieved %s",this->name_.c_str(), schedule_slot_id, events.c_str());
    this->parent_->send_notification_to_ha("Text Rxed","Have rx a text schedule","103");
}

void RTCScheduler::on_schedule_recieved(int schedule_slot_id,  std::vector<int> days ,std::vector<int> hours ,std::vector<int> minutes, std::vector<std::string> &actions) {
    ESP_LOGD(TAG, "%s Schedule Slot %d   recieved",this->name_.c_str(), schedule_slot_id);
    ESP_LOGD(TAG, "Entries Count - Day:%d, Hours: %d Mins:%d, Actions: %d",days.size(),hours.size(), minutes.size(), actions.size() );
     send_log_message_to_HA("error","The test message from Controller","ESPHome: boiler_controller");
    // Verify and write data to eeprom
   
    // Setup next schedule next event per switch
}

void  RTCScheduler::on_schedule_erase_recieved(int schedule_slot_id){
    ESP_LOGD(TAG, "%s Schedule Slot %d  erase recieved",this->name_.c_str() ,schedule_slot_id);
    // Mark slot as inactive and clear data
}
void  RTCScheduler::on_erase_all_schedules_recieved(){
        ESP_LOGD(TAG, "%s Erase all schedules recieved",this->name_.c_str());
        // Mark all slots as inactive and clear data and disable schedule loop
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
   this->item_mode_select_->configure_item(item_slot_number, item_sw,item_sw_id,item_status, item_next_event,  item_on_indicator);
}
void RTCScheduler::Test_Set_Slot_Valid(uint8_t item_slot_number, bool valid)
{
  RTCSchedulerItemMode_Select* sched_item = get_scheduled_item_from_slot(item_slot_number);
  if(sched_item != nullptr){
    sched_item->set_item_schedule_valid(valid);
    ESP_LOGD(TAG, "setting slot %d", item_slot_number);
  }
}
void RTCScheduler::Test_Set_Slot_Sw(uint8_t item_slot_number, bool sw_state)
{
  RTCSchedulerItemMode_Select* sched_item = get_scheduled_item_from_slot(item_slot_number);
  if(sched_item != nullptr)
    sched_item->set_scheduled_item_state(sw_state);
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
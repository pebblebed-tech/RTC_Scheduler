#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "rtc_scheduler_hub.h"

namespace esphome {
namespace rtc_scheduler {
  static const char *TAG = "rtc_scheduler_hub";
/* RTCSchedulerHub::RTCSchedulerHub()
{
}
RTCSchedulerHub::RTCSchedulerHub(const std::string &name) : EntityBase(name)
{
} */
void RTCSchedulerHub::setup()
{
  std::string service_name;
  service_name = "send_schedule";
  register_service(&RTCSchedulerHub::on_schedule_recieved, service_name,
                   {"scheduler_id","schedule_slot_id",  "days", "hours","minutes","actions"});
  
  service_name = "erase_schedule";
  register_service(&RTCSchedulerHub::on_schedule_erase_recieved, service_name,{"scheduler_id","schedule_slot_id"});

  service_name = "erase_all_schedules";
  register_service(&RTCSchedulerHub::on_erase_all_schedules_recieved, service_name,{"scheduler_id"}); 

  service_name = "send_text_schedule";             
  register_service(&RTCSchedulerHub::on_text_schedule_recieved, service_name,{"scheduler_id","schedule_slot_id", "events"}); 
}
void RTCSchedulerHub::loop()
{
}
void RTCSchedulerHub::dump_config()
{
  ESP_LOGCONFIG(TAG, "Scheduler Hub");
  ESP_LOGCONFIG(TAG, "RTC Scheduler Hub -- %s", this->name_.c_str());

}
void RTCSchedulerHub::add_controller(RTCScheduler *schedule_controller)
{

  schedule_controller->set_parent(this);
  schedule_controller->set_storage(this->storage_);
  this->schedule_controllers_.push_back(schedule_controller);

}
// service callbacks

void RTCSchedulerHub::on_text_schedule_recieved(std::string scheduler_id,int schedule_slot_id, std::string events) {
    ESP_LOGD(TAG, "Text Schedule %s Slot %d   recieved", scheduler_id.c_str(),schedule_slot_id);
    RTCScheduler *the_scheduler = this->get_scheduler(scheduler_id);
        if (the_scheduler != nullptr)
          the_scheduler->on_text_schedule_recieved(schedule_slot_id,events);
}
void RTCSchedulerHub::on_schedule_recieved(std::string scheduler_id,int schedule_slot_id, std::vector<int> days ,std::vector<int> hours ,std::vector<int> minutes, std::vector<std::string> actions) {
    ESP_LOGD(TAG, "Scheduler %s Slot %d schedule recieved",scheduler_id.c_str(), schedule_slot_id);
    RTCScheduler *the_scheduler = this->get_scheduler(scheduler_id);
        if (the_scheduler != nullptr)
          the_scheduler->on_schedule_recieved(schedule_slot_id,days,hours,minutes,actions);
}

void  RTCSchedulerHub::on_schedule_erase_recieved(std::string scheduler_id,int schedule_slot_id){
    ESP_LOGD(TAG, "%s Schedule Slot %d  erase recieved",scheduler_id.c_str(), schedule_slot_id);
    RTCScheduler *the_scheduler = this->get_scheduler(scheduler_id);
        if (the_scheduler != nullptr)
          the_scheduler->on_schedule_erase_recieved(schedule_slot_id);
}
void  RTCSchedulerHub::on_erase_all_schedules_recieved(std::string scheduler_id){
        ESP_LOGD(TAG, "Hub Erase all schedules recieved sched %s",scheduler_id.c_str());
        RTCScheduler *the_scheduler = this->get_scheduler(scheduler_id);
        if (the_scheduler != nullptr)
          the_scheduler->on_erase_all_schedules_recieved();

}

// send event back to HA 
void RTCSchedulerHub::send_event_to_ha(std::string event_str){
    fire_homeassistant_event(event_str.c_str());
   }

// send notification back to HA
void RTCSchedulerHub::send_notification_to_ha(std::string title, std::string message,std::string id ){
    call_homeassistant_service("persistent_notification.create", {
                                                    {"title", title.c_str()},
                                                    {"message", message.c_str()},
                                                    {"notification_id", id.c_str()},
                                                }); 
}
// send log message back to HA 
void RTCSchedulerHub::send_log_message_to_ha(std::string level, std::string logMessage, std::string sender)
{
       call_homeassistant_service("system_log.write", {
                                                    {"message", logMessage.c_str()},
                                                    {"level",   level.c_str()},
                                                    {"logger", sender.c_str()},
                                                });
        
//        fire_homeassistant_event("esphome.something_happened", {
//      {"my_value", "500"},});
}
void RTCSchedulerHub::set_storage(external_eeprom::ExtEepromComponent *storage)
{
  this->storage_ = storage;
}
RTCScheduler *RTCSchedulerHub::get_scheduler(std::string &scheduler_id)
{
//        for (auto &controller : this->schedule_controllers_)
 /*      for (auto &controller : this->schedule_controllers_)
        { // lets find the right scheduler

          if (controller->get_object_id() == scheduler_id)
          {
            return controller;
          }
        } */
        return nullptr;
}
void RTCSchedulerHub::display_storage_status()
{
  std::string scheduler_id;
  bool status;
  /* for (auto &controller : this->schedule_controllers_)
        { 
          
         scheduler_id = controller->get_object_id();
         status = controller->get_storage_status();
         ESP_LOGD(TAG, "Storage for %s is %d",scheduler_id.c_str(),status);
        } */
}
} // namespace rtc_scheduler
}  // namespace esphome
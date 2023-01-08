#pragma once
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/core/component.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/core/hal.h"
#include "rtc_scheduler.h"

namespace esphome {
namespace rtc_scheduler {
class RTCScheduler;                       // the scheduler controller
class RTCSchedulerHub;                    // this component

class RTCSchedulerHub : public Component, public api::CustomAPIDevice,  public EntityBase {
 public:
   RTCSchedulerHub();
   RTCSchedulerHub(const std::string &name);  
   void setup() override;
   void loop() override;
   void dump_config() override;
   float get_setup_priority() const override { return setup_priority::HARDWARE_LATE; }
   void add_controller(RTCScheduler *schedule_controller);
   void on_text_schedule_recieved(std::string scheduler_id,int schedule_slot_id, std::string events);
   void on_schedule_recieved(std::string scheduler_id,int schedule_slot_id, std::vector<int> days ,std::vector<int> hours ,std::vector<int> minutes, std::vector<std::string> actions);
   void on_schedule_erase_recieved(std::string scheduler_id,int schedule_slot_id);
   void on_erase_all_schedules_recieved(std::string scheduler_id);
   void send_log_message_to_ha(std::string level, std::string logMessage, std::string sender);
   void send_event_to_ha(std::string event_str);
   void send_notification_to_ha(std::string title, std::string message,std::string id );


 protected:
   RTCScheduler* get_scheduler(std::string &scheduler_id);
   std::vector<RTCScheduler *> schedule_controllers_;
};



} // namespace rtc_scheduler
}  // namespace esphome

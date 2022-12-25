#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "rtc_scheduler.h"

namespace esphome {
namespace rtc_scheduler {



template<typename... Ts> class ShutdownAction : public Action<Ts...> {
 public:
  explicit ShutdownAction(RTCScheduler *a_rtc_scheduler) : rtc_scheduler_(a_rtc_scheduler) {}

  void play(Ts... x) override { this->rtc_scheduler_->shutdown_schedule_controller(); }

 protected:
  RTCScheduler *rtc_scheduler_;
};



template<typename... Ts> class StartAction : public Action<Ts...> {
 public:
  explicit StartAction(RTCScheduler *a_rtc_scheduler) : rtc_scheduler_(a_rtc_scheduler) {}

  void play(Ts... x) override { this->rtc_scheduler_->resume_or_start_schedule_controller(); }

 protected:
  RTCScheduler *rtc_scheduler_;
};

}  // namespace rtc_scheduler
}  // namespace esphome

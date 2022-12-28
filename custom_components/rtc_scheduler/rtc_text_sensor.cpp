#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "rtc_text_sensor.h"
namespace esphome {
namespace rtc_scheduler {
static const char *TAG = "rtc_scheduler_text_sensor";
void RTCSchedulerTextSensor::dump_config()
{
  LOG_TEXT_SENSOR("  ", "TextSensor ", this);
}
} // namespace rtc_scheduler
}  // namespace esphome
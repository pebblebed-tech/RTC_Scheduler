#include "esphome.h"

using namespace std;

// Declare functions before calling them.


int   get_schedule_cnt(const std::vector<std::string> & schedule) {
     return schedule.size();
}

 std::string  flatten_schedule(const std::vector<std::string> & schedule) {  
  std::string s;
  for (std::vector<std::string>::const_iterator i = schedule.begin(); i != schedule.end(); ++i)
       s += *i;
  return s;

}


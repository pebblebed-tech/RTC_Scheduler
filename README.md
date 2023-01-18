# RTC_Scheduler
> :warning:  **Note This is a work in progress and is not fully working yet.**

This is a realtime weekly scheduler component for ESPHome that allows the scheduling of switches independantly of Home Assistant. This ensures that scheduled switched equipment will always opperate regardless of the availability of Home Assistant and the network. The schedule is supplied by Home Assistant via services this is either text string or complex data structure.
The scheduler has the following structure
-Scheduler Hub 
  -- Schedule Controller (1.. n)
      --- Switches (1..n)
Each scheduled switch can have multiple timing events that define on and off times upto ``` max_events_per_switch ```.
Day 0 - 7, Hour 0 - 23, Minutes 0 - 59, State (On / Off)
Currently there are 2 ways to get the schedule from Home Assistant
* Text string supplied by service call. This is limited 256 characters so given it takes 9 characters to describe an event this equates to 28 events in a week.
* A service call that takes a complex structure that can be used with the supplied python script and a HACS scheduler. See https://github.com/nielsfaber/scheduler-component
* In the future a version a  built in home assistant scheduler helper will be produced and will call the data service. Fingers crossed 

## Addtional Hardware required 
To acheive this the software requires a couple of hardware components (DS3231 & 24LCxx E2). The easiest way to add these devices is via a ZS-042 (https://www.google.com/search?q=zs-04). These devices are I2C so the device configuration needs setup I2C with the appropiate pin configurations for your device.

> :warning:  **Note there is a modification needed to the module. Remove the 1N4148 and/or the 200R resistor and everything is fine.**

## Example configuration
```
# See further details below
i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true    
  time:
  - platform: ds1307
    id: rtc_time
    # repeated synchronization is not necessary unless the external RTC
    # is much more accurate than the internal clock
    update_interval: never
ext_eeprom_component:
  id: ext_eeprom_component_1
  address: 0x57 
  pollWriteComplete: true
  writeTime: 5
  pageSize: 32
  memorySize: 4096
rtc_scheduler:
  - id: scheduler_hub
    schedulers:
      - id: scheduler_1
        storage: ext_eeprom_component_1
        storage_offset: 1000
        storage_size: 32768
        max_events_per_switch: 56
        schedule_controller_status_id: "Heater Sheduler Status"
        scheduler_mode: "Heater Controller Mode"
        scheduler_ind: "Heater Indicator"
        main_switch: 
          name: "Heater Scheduler"
          on_turn_on:
            then:
              - logger.log: "Heater Turned On by action!"
        switches:
          - scheduled_mode: "Element 1 Mode"
            scheduled_switch: 
              name: "Element 1"
              on_turn_on:
                then:
                - logger.log: "element 1 Turned On by action!"
            scheduled_switch_id: relay3
            scheduler_slot: 1
            scheduled_status: "Element 1 Status"
            scheduled_next_event_text: "Element 1 Next Event"
            scheduled_indicator: "Element 1 Indicator"
          - scheduled_switch: "Element 2"
            scheduled_switch_id: relay4 
            scheduler_slot: 2  
            scheduled_status: "Element 2 Status"
            scheduled_mode: "Element 2 Mode"
            scheduled_indicator: "Element 2 Indicator"    
```
## Configuration variables:
**id** *(Required)* Manually specify the ID used for code generation.

**storage** *(**Required**, string)* : This is the ID of the storage component.

**storage_offset** *(**Required**, int)* : This is offset in bytes from the start of the E2 device useful if you have multiple schedulers configured.

**storage_size** *(**Required**, int)* : This is size of the E2 device in bytes.

**max_events_per_switch** *(**Required**, int)* : This is number of events each slot can hold.

**main_switch** *(**Required**, string)*:  The name for the scheduler controller's main switch as it will appear in the front end. This switch, when turned on, Allows the scheduler to opperate either from a schedule or from manual controls (select) from Home Assistant.

**switches** *(**Required**, list)* : A list of scheduled items the scheduler can control. Each item consists of:

- **scheduled_switch** *(Optiopnal, string)*: Name of a virtual switch that can be controlled by the scheduler. This can use the standard ESPHome switch on_turn_on & on_turn_off functions.
- **scheduled_id** *(Optional, ID)*: This is the :ref:`switch <config-switch>` component. Typically this would be a :doc:`GPIO switch <switch/gpio>` wired to control a relay or other switching device. It is not recommended to expose this switch to the front end.
- **scheduled_slot** *(**Required**, int)*: This is the slot number for the scheduled switch. This is used to store the events for the schedule item in the E2 device. The range is 1 to 255. It should be note that these must be unique per schedule controller
- **scheduled_mode** *(**Required, string**)*: Name of the select that is added to the home assistant frontend and used to control the scheduled item. Option are:-

* ```Manual Off```, The schedule item is permentley off. Until a another mode is selected.

* ```Early Off```,  The schedule item is in ```Auto``` and can be set to off before the scheduled time. Once the scheduled time is reached the mode will revert back to ```Auto```

* ```Auto```,  The scheduled item is turn on and off as per the supplied schedule.

* ```Manual On```,  The schedule item is permentley on. Until a another mode is selected.

* ```Boost On```,  The schedule item is in ```Auto``` and can be extended on past the scheduled off time. Once the next scheduled on time is reached the mode will revert back to ```Auto```. Boost can be canceled at any time by selecting ```Auto``` again. At which point the scheduled irtem will adopt the correct state for that time period from the supplied schedule.

- **scheduled_status** *(Optiopnal, string)*: Name of a text sensor that displays the current state of the scheduled item in Home Assistant. Useful for debugging schedules.

- **scheduled_indicator** *(Optiopnal, string)*: Name of a binary sensor that reflects the current state of the scheduled item in Home Assistant.

## Services presented to Home Assistant
The scheduler presents a set of services to Home Assistant
Each service is prefixed by *"esphome."* and the *device_name* and *scheduler_name* from the config in the yaml. As an example ```ESPHome: schedule_test_pump_scheduler_send_schedule"```

The services are as follows:-
### send_schedule
Send a schedule for a slot.
#### parameters
- ```schedule_slot_id int``` - The slot to be updated.
- ```std::vector<int> days``` - Array ints representing days (0-6)
- ```std::vector<int> hours``` - Array ints representing hours (0-23)
- ```std::vector<int> minutes``` - Array ints representing hours (0-59)
- ```std::vector<std::string> action``` - Currently "ON" or "OfFF" to be expanded to manage state. 

### send_schedule_text
Send a schedule for a slot.
#### parameters
- ```std::string ``` - Takes the form of Slot ID, Event, Event, ..... 
- Event encodes as this DHHMMState where state is either ON or OFF
- Example "1,61140ON,61400OFF
### erase_schedule
Erase a single slot schedule
#### parameters
- ```schedule_slot_id int``` - The slot to be deleted
### erase_all_schedules 
No parameters - Erase all slots that belong that scheduler

## Storage Allocation
Each schedule controller defines the starting address of its event storage using the ```storage_offset``` configuration entry E.G. Offset plus location. 

### Schedule Controller Usage
| Bytes   | Description                | Notes                         |
| ---------- | -------------------------- | ----------------------------- |
| 0:3       | Controller Storage Setup | Used to initialise E2 for schedule storage. Once configured these are set to known value |
| 4:--      | Slots with events | See the slot definition below |

The each scheduled item has it own slot with header information as per below. 
### Slot Usage
| Bytes   | Description                | Notes                         |
| ---------- | -------------------------- | ----------------------------- |
| 0:1       | Slot Valid | Used to initialise the schedule storage. Once configured these are set to known value. If the slot is erased it is setback to non valid value |
| 2:3      | Slots CRC Checksum | This is calculated on writing the slot data and is checked  before a new schedule is written to the slot (Protects Flash Life) |
| 4:--      | Event Storage | See definition of a event below |

### Event Format
Each event is consists 16bit word (2 bytes)
| Bits   | Description                | Notes                         |
| ---------- | -------------------------- | ----------------------------- |
| 0:13       | Event Time | In seconds, the maximum value is  10080 *EG 7 Days x 24 Hours x 60 Mins*|
| 14     |  Event state | 1 for on, 0 for off   |
| 15     | Valid Event | This has to be set to true for the event to be valid. It should be noted that on a new schedule being recieved un-used events are set to invalid EG upto value supplied in *max_events_per_switch*  |

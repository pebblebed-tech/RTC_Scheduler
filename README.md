# RTC_Scheduler
This is a realtime scheduler component for ESPHome that allows the scheduling of switches independantly of Home Assistant. To acheive this it requires a couple of hardware components (DS3231 & 24LCxx E2). the easiest way to add these devices is via a ZS-042 (https://www.google.com/search?q=zs-04).
Note there is a modification needed to the module. Remove the 1N4148 and/or the 200R resistor and everything is fine.
```
# Example configuration
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
  - id: scheduler_1
    storage: ext_eeprom_component_1
    storage_offset: 1000
    storage_size: 32768
    max_events_per_switch: 56
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

**storage_offset** *(**Required**, int)* : This is offset from the start of the E2 device useful if you have multiple schedulers configured.

**storage_size** *(**Required**, int)* : This is size of the E2 device in bits.

**main_switch** *(**Required**, string)*:  The name for the scheduler controller's main switch as it will appear in the front end. This switch, when turned on, Allows the scheduler to opperate either from a schedule or from manual controls (select) from Home Assistant.

**switches** *(**Required**, list)* : A list of scheduled items the scheduler can control. Each item consists of:

- **scheduled_switch** *(Optiopnal, string)*: Name of a virtual switch that can be controlled by the scheduler. This can use the standard ESPHome switch on_turn_on & on_turn_off functions.
- **scheduled_id** *(Optional, ID)*: This is the :ref:`switch <config-switch>` component. Typically this would be a :doc:`GPIO switch <switch/gpio>` wired to control a relay or other switching device. It is not recommended to expose this switch to the front end
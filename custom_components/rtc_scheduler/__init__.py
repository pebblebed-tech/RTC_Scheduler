import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID
from esphome.automation import maybe_simple_id
from esphome.components import ext_eeprom_component
from esphome.components import switch
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    
)
AUTO_LOAD = ["switch"]
CODEOWNERS = ["@pebblebed-tech"]
DEPENDENCIES = ['api']

CONF_EXT_EEPROM ='storage'
CONF_EXT_EEPROM_OFFSET ='storage_offset'
CONF_EXT_EEPROM_SIZE ='storage_size'
CONF_MAIN_SWITCH = "main_switch"
CONF_MAX_EVENTS_PER_SW = "max_events_per_switch"
CONF_SCHEDULED_SWITCH = "scheduled_switch"
CONF_SCHEDULED_SWITCH_ID = "scheduled_switch_id"
CONF_SCHEDULER_SLOT = "scheduler_slot"

CONF_SWITCHES = "switches"

rtc_scheduler_ns = cg.esphome_ns.namespace('rtc_scheduler')
RTCScheduler = rtc_scheduler_ns.class_('RTCScheduler', cg.Component)
SchedulerControllerSwitch = rtc_scheduler_ns.class_(
    "RTCSchedulerControllerSwitch", switch.Switch, cg.Component
)
ShutdownAction = rtc_scheduler_ns.class_("ShutdownAction", automation.Action)
StartAction = rtc_scheduler_ns.class_("StartAction", automation.Action)

# TODO validate the slot data in final validate routine
# TODO Validate data is in real range
def validate_scheduler(config):
    for scheduler_controller_index, scheduler_controller in enumerate(config):
        requirements = [
                CONF_MAIN_SWITCH,
                CONF_EXT_EEPROM,
                CONF_EXT_EEPROM_OFFSET,
                CONF_EXT_EEPROM_SIZE,
                CONF_MAX_EVENTS_PER_SW,
            ]

        for config_item in requirements:
                if config_item not in scheduler_controller:
                    raise cv.Invalid(
                        f"{config_item} is a required option for {scheduler_controller_index}"
                    )
        if not scheduler_controller[CONF_SWITCHES]:
            raise cv.Invalid(
                        f"A controller must have at least one switch, Controller {scheduler_controller_index}"
            )
        slots = []    
        for value in scheduler_controller[CONF_SWITCHES]:
            if slots is None:
                slots.append(value[CONF_SCHEDULER_SLOT])
            else:
                if value[CONF_SCHEDULER_SLOT] not in slots:
                    slots.append(value[CONF_SCHEDULER_SLOT]) 
                else:
                    raise cv.Invalid(
                        f"Slot number {value[CONF_SCHEDULER_SLOT]} cannot be used more than once they must be unique"  
                    )              
    return config


SCHEDULER_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(RTCScheduler),
    }
)

SCHEDULER_SWITCH_SCHEMA = cv.Schema(
    {
        
        
        cv.Required(CONF_SCHEDULED_SWITCH): cv.maybe_simple_value(
            switch.switch_schema(SchedulerControllerSwitch),
            key=CONF_NAME,
        ),
        cv.Required(CONF_SCHEDULER_SLOT): cv.uint8_t,
        cv.Required(CONF_SCHEDULED_SWITCH_ID): cv.use_id(switch.Switch),
    }
)
SCHEDULER_CONTROLLER_SCHEMA = cv.Schema(
    {

        cv.GenerateID(): cv.declare_id(RTCScheduler),
        cv.Required(CONF_EXT_EEPROM): cv.use_id(ext_eeprom_component.ExtEepromComponent),
        cv.Required(CONF_EXT_EEPROM_OFFSET): cv.uint16_t,
        cv.Required(CONF_MAX_EVENTS_PER_SW): cv.uint16_t,
        cv.Required(CONF_EXT_EEPROM_SIZE): cv.uint32_t,
        #cv.Required(CONF_SCHEDULE_SWITCH_CNT): cv.uint8_t,       
        cv.Required(CONF_MAIN_SWITCH): cv.maybe_simple_value(
            switch.switch_schema(SchedulerControllerSwitch),
            key=CONF_NAME,
        ),
        cv.Required(CONF_SWITCHES): cv.ensure_list(SCHEDULER_SWITCH_SCHEMA),
    }
).extend(cv.ENTITY_BASE_SCHEMA)
CONFIG_SCHEMA = cv.All(
    cv.ensure_list(SCHEDULER_CONTROLLER_SCHEMA),
    validate_scheduler,
)
#CONFIG_SCHEMA = cv.Schema(
#    {
#        
#    }
#).extend(cv.COMPONENT_SCHEMA)


@automation.register_action(
    "rtc_scheduler.resume_or_start_schedule_controller", StartAction, SCHEDULER_ACTION_SCHEMA
)
@automation.register_action(
    "rtc_scheduler.shutdown_schedule_controller", ShutdownAction, SCHEDULER_ACTION_SCHEMA
)
async def scheduler_simple_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

async def to_code(config):
    #var = cg.new_Pvariable(config[CONF_ID])
    #await cg.register_component(var, config)
    
    for scheduler_controller in config:
        
        var = cg.new_Pvariable(
            scheduler_controller[CONF_ID],
            scheduler_controller[CONF_MAIN_SWITCH][CONF_NAME],
            )
        await cg.register_component(var, scheduler_controller)
        store = await cg.get_variable(scheduler_controller[CONF_EXT_EEPROM])
        cg.add(var.set_storage(store))
        cg.add(var.set_Storage_Offset(
            scheduler_controller[CONF_EXT_EEPROM_OFFSET]))
        cg.add(var.set_Events_Per_Switch(
            scheduler_controller[CONF_MAX_EVENTS_PER_SW]))
        
        sw_var = await switch.new_switch(scheduler_controller[CONF_MAIN_SWITCH])
        await cg.register_component(sw_var, scheduler_controller[CONF_MAIN_SWITCH])
        cg.add(var.set_controller_main_switch(sw_var))



    #     for sch_switch in scheduler_controller[CONF_SWITCHES]:
    #         sch_switch_var = await switch.new_switch(sch_switch[CONF_SCHEDULED_SWITCH])
    #         await cg.register_component(sch_switch_var, sch_switch[CONF_SCHEDULED_SWITCH])
    #         cg.add(var.add_switch(sch_switch_var))

    # for scheduler_controller in config:
    #     var = await cg.get_variable(scheduler_controller[CONF_ID])
    #     for switch_index, sch_switch in enumerate(scheduler_controller[CONF_SWITCHES]):
    #         if CONF_SCHEDULED_SWITCH_ID in sch_switch:
    #             switch_obj = await cg.get_variable(sch_switch[CONF_SCHEDULED_SWITCH_ID])
    #             cg.add(
    #                 var.configure_scheduled_switch(
    #                     switch_index, switch_obj
                        
    #                 )
    #             )
         

    for scheduler_controller in config:
        var = await cg.get_variable(scheduler_controller[CONF_ID])
        for controller_to_add in config:
            if scheduler_controller[CONF_ID] != controller_to_add[CONF_ID]:
                cg.add(
                    var.add_controller(
                        await cg.get_variable(controller_to_add[CONF_ID])
                    )
                )
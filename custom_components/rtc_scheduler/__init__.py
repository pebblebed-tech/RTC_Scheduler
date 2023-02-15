import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID
from esphome.automation import maybe_simple_id
from esphome.components import ext_eeprom_component
from esphome.components import switch
from esphome.components import text_sensor
from esphome.components import select
from esphome.components import binary_sensor
from esphome.cpp_generator import MockObjClass
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_ICON,
    CONF_ENTITY_CATEGORY,
)
AUTO_LOAD = ["switch" , "text_sensor", "select", "binary_sensor"]
CODEOWNERS = ["@pebblebed-tech"]
DEPENDENCIES = ['api']

CONF_EXT_EEPROM ='storage'
CONF_EXT_EEPROM_OFFSET ='storage_offset'
CONF_EXT_EEPROM_SIZE ='storage_size'
CONF_MAIN_SWITCH = "main_switch"
CONF_CONTROLLER_STATUS_ID = "schedule_controller_status_id"
CONF_MAX_EVENTS_PER_SW = "max_events_per_switch"
CONF_SCHEDULER_SELECT = "scheduler_mode"
CONF_SCHEDULER_IND = "scheduler_ind"
CONF_SCHEDULERS = "schedulers"

CONF_SWITCHES = "switches"
CONF_SCHEDULED_SWITCH = "scheduled_switch"
CONF_SCHEDULED_SWITCH_ID = "scheduled_switch_id"
CONF_SCHEDULER_SLOT = "scheduler_slot"
CONF_SCHEDULED_MODE = "scheduled_mode"
CONF_SCHEDULED_IND = "scheduled_indicator"
CONF_SCHEDULED_STATUS = "scheduled_status"
CONF_SCHEDULED_NEXT_EVENT = "scheduled_next_event_text"

SCHEDULED_ITEM_MODE_OPTIONS = [
    "Manual Off",
    "Early Off",
    "Auto",
    "Manual On",
    "Boost On"
]
_UNDEF = object()
rtc_scheduler_hub_ns = cg.esphome_ns.namespace('rtc_scheduler')
RTCSchedulerHub = rtc_scheduler_hub_ns.class_('RTCSchedulerHub', cg.Component)
rtc_scheduler_ns = cg.esphome_ns.namespace('rtc_scheduler')
RTCScheduler = rtc_scheduler_ns.class_('RTCScheduler', cg.Component)

SchedulerControllerSwitch = rtc_scheduler_ns.class_(
    "RTCSchedulerControllerSwitch", switch.Switch, cg.Component)
SchedulerTextSensor = rtc_scheduler_ns.class_(
    "RTCSchedulerTextSensor", text_sensor.TextSensor, cg.Component)
ScheduledItemSelect = rtc_scheduler_ns.class_(
    "RTCSchedulerItemMode_Select", select.Select, cg.Component)
ShutdownAction = rtc_scheduler_ns.class_("ShutdownAction", automation.Action)
StartAction = rtc_scheduler_ns.class_("StartAction", automation.Action)
rtc_scheduler_ns = cg.esphome_ns.namespace('rtc_scheduler')
RTCScheduler = rtc_scheduler_ns.class_('RTCScheduler', cg.Component)

# SchedulerControllerSwitch = rtc_scheduler_ns.class_("RTCSchedulerControllerSwitch", switch.Switch, cg.Component)
# SchedulerTextSensor=rtc_scheduler_ns.class_("RTCSchedulerTextSensor", text_sensor.TextSensor, cg.Component)
# ScheduledItemSelect = rtc_scheduler_ns.class_(
#     "RTCSchedulerItemMode_Select", select.Select, cg.Component)
# ShutdownAction = rtc_scheduler_ns.class_("ShutdownAction", automation.Action)
# StartAction = rtc_scheduler_ns.class_("StartAction", automation.Action)
def select_schema(
    class_: MockObjClass=_UNDEF,
    *,
    icon: str=_UNDEF,
    entity_category: str=_UNDEF,
) -> cv.Schema:
    schema = select.SELECT_SCHEMA
    if class_ is not _UNDEF:
        schema = schema.extend({cv.GenerateID(): cv.declare_id(class_)})
    if icon is not _UNDEF:
        schema = schema.extend({cv.Optional(CONF_ICON, default=icon): cv.icon})
    if entity_category is not _UNDEF:
        schema = schema.extend(
            {
                cv.Optional(
                    CONF_ENTITY_CATEGORY, default=entity_category
                ): cv.entity_category
            }
        )
    return schema
# TODO validate the slot data in final validate routine
# TODO Validate data is in real range
def validate_scheduler(config):
    for  scheduler_hub in config:
        #for scheduler_controller_index, scheduler_controller in enumerate(config):
        scheduler_controller_index = 0
        for scheduler_controller in scheduler_hub[CONF_SCHEDULERS]:
            requirements = [
                    CONF_MAIN_SWITCH,
                    CONF_EXT_EEPROM_OFFSET,
                    CONF_MAX_EVENTS_PER_SW,
                    
                ]

            for config_item in requirements:
                    if config_item not in scheduler_controller:
                        raise cv.Invalid(
                            f"{config_item} is a required option for {scheduler_controller_index}"
                        )

            if not scheduler_controller[CONF_SWITCHES]:
                raise cv.Invalid(
                            f"A controller must have at least one schedule  item, Controller {scheduler_controller_index}"
                )
            slots = []    
            for item in scheduler_controller[CONF_SWITCHES]:
                if slots is None:
                    slots.append(item[CONF_SCHEDULER_SLOT])
                else:
                    if item[CONF_SCHEDULER_SLOT] not in slots:
                        slots.append(item[CONF_SCHEDULER_SLOT]) 
                    else:
                        raise cv.Invalid(
                            f"Slot number {item[CONF_SCHEDULER_SLOT]} cannot be used more than once they must be unique"  
                        )
                if ((not CONF_SCHEDULED_SWITCH_ID in item) and (not CONF_SCHEDULED_SWITCH in item)):
                    raise cv.Invalid(
                        f"Slot number {item[CONF_SCHEDULER_SLOT]} must have either a scheduled_switch_id or scheduled_switch"
                    )
            scheduler_controller_index = scheduler_controller_index +1
    return config




SCHEDULER_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(RTCScheduler),
    }
)

SCHEDULER_SWITCH_SCHEMA = cv.Schema(
    {
        
        

        cv.Required(CONF_SCHEDULER_SLOT): cv.uint8_t,
        cv.Optional(CONF_SCHEDULED_SWITCH_ID): cv.use_id(switch.Switch),
        
        cv.Optional(CONF_SCHEDULED_NEXT_EVENT): cv.maybe_simple_value(
            text_sensor.text_sensor_schema(SchedulerTextSensor),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_SCHEDULED_STATUS): cv.maybe_simple_value(
            text_sensor.text_sensor_schema(SchedulerTextSensor),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_SCHEDULED_IND): cv.maybe_simple_value(
            binary_sensor.binary_sensor_schema().extend(),
            key=CONF_NAME,
        ),
        cv.Required(CONF_SCHEDULED_MODE): cv.maybe_simple_value(
            select_schema(ScheduledItemSelect),
            key=CONF_NAME,
        ),
        cv.Optional(CONF_SCHEDULED_SWITCH): cv.maybe_simple_value(
            switch.switch_schema(SchedulerControllerSwitch),
            key=CONF_NAME,
        ),
    }
)


SCHEDULER_CONTROLLER_SCHEMA = cv.Schema(
    {

        cv.GenerateID(): cv.declare_id(RTCScheduler),
        cv.Required(CONF_EXT_EEPROM_OFFSET): cv.uint16_t,
        cv.Required(CONF_MAX_EVENTS_PER_SW): cv.uint16_t,

        
        cv.Required(CONF_SCHEDULER_IND): cv.maybe_simple_value(
            binary_sensor.binary_sensor_schema().extend(),
            key=CONF_NAME,
        ),
        cv.Required(CONF_SCHEDULER_SELECT): cv.maybe_simple_value(
            select_schema(ScheduledItemSelect),
            key=CONF_NAME,
        ),
        cv.Required(CONF_MAIN_SWITCH): cv.maybe_simple_value(
            switch.switch_schema(SchedulerControllerSwitch),
            key=CONF_NAME,
        ),
        
        cv.Required(CONF_CONTROLLER_STATUS_ID): cv.maybe_simple_value(
            text_sensor.text_sensor_schema(SchedulerTextSensor),
            key=CONF_NAME,
        ),
        cv.Required(CONF_SWITCHES): cv.ensure_list(SCHEDULER_SWITCH_SCHEMA),
    }
).extend(cv.ENTITY_BASE_SCHEMA)

SCHEDULER_HUB_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RTCSchedulerHub),
        cv.Required(CONF_EXT_EEPROM): cv.use_id(ext_eeprom_component.ExtEepromComponent),
        cv.Required(CONF_SCHEDULERS): cv.ensure_list(SCHEDULER_CONTROLLER_SCHEMA),
    }
).extend(cv.ENTITY_BASE_SCHEMA)
CONFIG_SCHEMA = cv.All(
    cv.ensure_list(SCHEDULER_HUB_SCHEMA),
    validate_scheduler,
)



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
    for scheduler_hub in config:
        varh = cg.new_Pvariable(
            scheduler_hub[CONF_ID],
            #scheduler_controller[CONF_MAIN_SWITCH][CONF_NAME],
        )
        await cg.register_component(varh, scheduler_hub)
        store = await cg.get_variable(scheduler_hub[CONF_EXT_EEPROM])
        cg.add(varh.set_storage(store))
        for scheduler_controller in scheduler_hub[CONF_SCHEDULERS]:
    #for scheduler_controller in config:
        
            var = cg.new_Pvariable(
                scheduler_controller[CONF_ID],
                scheduler_controller[CONF_MAIN_SWITCH][CONF_NAME],
                )
            await cg.register_component(var, scheduler_controller)
#            store = await cg.get_variable(scheduler_controller[CONF_EXT_EEPROM])
#            cg.add(var.set_storage(store))
            cg.add(var.set_storage_offset(
                scheduler_controller[CONF_EXT_EEPROM_OFFSET]))
            cg.add(var.set_events_per_switch(
                scheduler_controller[CONF_MAX_EVENTS_PER_SW]))

            sens = await binary_sensor.new_binary_sensor(scheduler_controller[CONF_SCHEDULER_IND])
            cg.add(var.set_ind(sens))

            status_var = await text_sensor.new_text_sensor(scheduler_controller[CONF_CONTROLLER_STATUS_ID])
            await cg.register_component(status_var, scheduler_controller[CONF_CONTROLLER_STATUS_ID])

            sw_var = await switch.new_switch(scheduler_controller[CONF_MAIN_SWITCH])
            await cg.register_component(sw_var, scheduler_controller[CONF_MAIN_SWITCH])

            cg.add(var.set_main_switch_status(status_var))
            cg.add(var.set_controller_main_switch(sw_var))
        # Now create the scheduled items
            for sch_switch in scheduler_controller[CONF_SWITCHES]:
                if CONF_SCHEDULED_STATUS in sch_switch:
                    status_var = await text_sensor.new_text_sensor(sch_switch[CONF_SCHEDULED_STATUS])
                    await cg.register_component(status_var, sch_switch[CONF_SCHEDULED_STATUS])
                if CONF_SCHEDULED_NEXT_EVENT in sch_switch:
                    next_var = await text_sensor.new_text_sensor(sch_switch[CONF_SCHEDULED_NEXT_EVENT])
                    await cg.register_component(next_var, sch_switch[CONF_SCHEDULED_NEXT_EVENT])

                if CONF_SCHEDULED_SWITCH in sch_switch:
                    sw_var = await switch.new_switch(sch_switch[CONF_SCHEDULED_SWITCH])
                    await cg.register_component(sw_var, sch_switch[CONF_SCHEDULED_SWITCH])
                
                if CONF_SCHEDULED_IND in sch_switch:
                    sens = await binary_sensor.new_binary_sensor(sch_switch[CONF_SCHEDULED_IND])

                if CONF_SCHEDULED_SWITCH_ID in sch_switch:
                    switch_id_var = await cg.get_variable(sch_switch[CONF_SCHEDULED_SWITCH_ID])
                

                selconf = sch_switch[CONF_SCHEDULED_MODE]
                mode_select=await select.new_select(selconf, options=SCHEDULED_ITEM_MODE_OPTIONS)
                await cg.register_component(mode_select, selconf)

                cg.add(var.add_scheduled_item(sch_switch[CONF_SCHEDULER_SLOT], sw_var, switch_id_var,  status_var, next_var, mode_select, sens ))
        # Add the the schedulers to the hub
        varh =  await cg.get_variable(scheduler_hub[CONF_ID])
        for scheduler_controller in scheduler_hub[CONF_SCHEDULERS]:
            cg.add(
                varh.add_controller(
                    await cg.get_variable(scheduler_controller[CONF_ID])
                )
            )
        # Now add the other schedulers
        for scheduler_controller in scheduler_hub[CONF_SCHEDULERS]:
            var = await cg.get_variable(scheduler_controller[CONF_ID])
            for controller_to_add in scheduler_hub[CONF_SCHEDULERS]:
                if scheduler_controller[CONF_ID] != controller_to_add[CONF_ID]:
                    cg.add(
                        var.add_controller(
                            await cg.get_variable(controller_to_add[CONF_ID])
                        )
                    )
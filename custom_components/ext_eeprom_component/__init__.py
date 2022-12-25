import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import CONF_ID

DEPENDENCIES = ['i2c']

CONF_I2C_ADDR = 0x57
CONF_EE_MEMSIZE = 'memorySize'
CONF_EE_PAGESIZE = 'pageSize'
CONF_EE_PAGEWRTTIME = 'writeTime'
CONF_EE_POLLWRITECOMPLETE = 'pollWriteComplete'

ext_eeprom_component_ns = cg.esphome_ns.namespace('ext_eeprom_component')
ExtEepromComponent = ext_eeprom_component_ns.class_('ExtEepromComponent', cg.Component, i2c.I2CDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ExtEepromComponent),
    cv.Required(CONF_EE_MEMSIZE): cv.uint32_t,
    cv.Required(CONF_EE_PAGESIZE): cv.uint16_t,
    cv.Required(CONF_EE_PAGEWRTTIME): cv.uint8_t,
    cv.Required(CONF_EE_POLLWRITECOMPLETE): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(CONF_I2C_ADDR))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield i2c.register_i2c_device(var, config)
    cg.add(var.setMemorySize(config[CONF_EE_MEMSIZE]))
    cg.add(var.setPageSize(config[CONF_EE_PAGESIZE]))
    cg.add(var.setPageWriteTime(config[CONF_EE_PAGEWRTTIME]))
    cg.add(var.setPollForWriteComplete(config[CONF_EE_POLLWRITECOMPLETE]))
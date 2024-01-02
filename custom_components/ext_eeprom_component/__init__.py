import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

CODEOWNERS = ["@pebblebed-tech"]
DEPENDENCIES = ['i2c']
MULTI_CONF = True

CONF_I2C_ADDR = 'address'
CONF_EE_MEMSIZE = 'memorySize'
CONF_EE_PAGESIZE = 'pageSize'
CONF_EE_PAGEWRTTIME = 'writeTime'
CONF_EE_POLLWRITECOMPLETE = 'pollWriteComplete'

ext_eeprom_component_ns = cg.esphome_ns.namespace('ext_eeprom_component')
ExtEepromComponent = ext_eeprom_component_ns.class_('ExtEepromComponent', cg.Component, i2c.I2CDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ExtEepromComponent),
    cv.Required(CONF_I2C_ADDR): cv.uint8_t,
    cv.Required(CONF_EE_MEMSIZE): cv.uint32_t,
    cv.Required(CONF_EE_PAGESIZE): cv.uint16_t,
    cv.Required(CONF_EE_PAGEWRTTIME): cv.uint8_t,
    cv.Required(CONF_EE_POLLWRITECOMPLETE): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(CONF_I2C_ADDR))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    cg.add(var.set_memory_size(config[CONF_EE_MEMSIZE]))
    cg.add(var.set_page_size(config[CONF_EE_PAGESIZE]))
    cg.add(var.set_page_write_time(config[CONF_EE_PAGEWRTTIME]))
    cg.add(var.set_poll_for_write_complete(config[CONF_EE_POLLWRITECOMPLETE]))
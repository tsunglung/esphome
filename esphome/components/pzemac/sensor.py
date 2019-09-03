import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import CONF_CURRENT, CONF_ID, CONF_POWER, CONF_VOLTAGE, \
    CONF_FREQUENCY, UNIT_VOLT, ICON_FLASH, UNIT_AMPERE, UNIT_WATT, UNIT_EMPTY, \
    ICON_POWER, CONF_ADDRESS

CONF_ENERGY = 'energy'
UNIT_WATTHOUR = 'Wh'
ICON_CURRRNT_AC = 'mdi:current-ac'
DEPENDENCIES = ['uart']

pzemac_ns = cg.esphome_ns.namespace('pzemac')
PZEMAC = pzemac_ns.class_('PZEMAC', cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(PZEMAC),

    cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(UNIT_VOLT, ICON_FLASH, 1),
    cv.Optional(CONF_CURRENT): sensor.sensor_schema(UNIT_AMPERE, ICON_CURRRNT_AC, 3),
    cv.Optional(CONF_POWER): sensor.sensor_schema(UNIT_WATT, ICON_POWER, 1),
    cv.Optional(CONF_FREQUENCY): sensor.sensor_schema(UNIT_EMPTY, ICON_CURRRNT_AC, 1),
    cv.Optional(CONF_ENERGY): sensor.sensor_schema(UNIT_WATTHOUR, ICON_FLASH, 0),
}).extend(cv.polling_component_schema('60s')).extend(uart.UART_DEVICE_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield uart.register_uart_device(var, config)

    if CONF_VOLTAGE in config:
        conf = config[CONF_VOLTAGE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_voltage_sensor(sens))
    if CONF_CURRENT in config:
        conf = config[CONF_CURRENT]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_current_sensor(sens))
    if CONF_POWER in config:
        conf = config[CONF_POWER]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_power_sensor(sens))
    if CONF_FREQUENCY in config:
        conf = config[CONF_FREQUENCY]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_frequency_sensor(sens))
    if CONF_ENERGY in config:
        conf = config[CONF_ENERGY]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_energy_sensor(sens))

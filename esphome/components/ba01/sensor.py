import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import CONF_TDS, CONF_TEMPERATURE, CONF_ID, ICON_WATER_PERCENT, \
    UNIT_PARTS_PER_MILLION, UNIT_CELSIUS, ICON_THERMOMETER

DEPENDENCIES = ['uart']

ba01_ns = cg.esphome_ns.namespace('ba01')
BA01Component = ba01_ns.class_('BA01Component', cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(BA01Component),
    cv.Required(CONF_TDS + "1"): sensor.sensor_schema(UNIT_PARTS_PER_MILLION, ICON_WATER_PERCENT, 0),
    cv.Required(CONF_TDS + "2"): sensor.sensor_schema(UNIT_PARTS_PER_MILLION, ICON_WATER_PERCENT, 0),
    cv.Optional(CONF_TEMPERATURE + "1"): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 0),
    cv.Optional(CONF_TEMPERATURE + "2"): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 0),
}).extend(cv.polling_component_schema('60s')).extend(uart.UART_DEVICE_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield uart.register_uart_device(var, config)

    if CONF_TDS + "1" in config:
        sens1 = yield sensor.new_sensor(config[CONF_TDS + "1"])
        cg.add(var.set_tds1_sensor(sens1))
        sens2 = yield sensor.new_sensor(config[CONF_TDS + "2"])
        cg.add(var.set_tds2_sensor(sens2))

    if CONF_TEMPERATURE + "1" in config:
        sens1 = yield sensor.new_sensor(config[CONF_TEMPERATURE + "1"])
        cg.add(var.set_temperature1_sensor(sens1))
        sens2 = yield sensor.new_sensor(config[CONF_TEMPERATURE + "2"])
        cg.add(var.set_temperature2_sensor(sens2))


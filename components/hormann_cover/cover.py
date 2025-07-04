import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import cover, binary_sensor

from esphome.const import (
    CONF_CLOSE_DURATION,
    CONF_CLOSE_ENDSTOP,
    CONF_OPEN_DURATION,
    CONF_OPEN_ENDSTOP,
    CONF_INTERVAL,
    CONF_TILT_ACTION,
    CONF_MAX_DURATION,
)
hormann_ns = cg.esphome_ns.namespace("hormann")
HormannCover = hormann_ns.class_("HormannCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = cover.cover_schema(HormannCover).extend(
    {
        cv.Required(CONF_TILT_ACTION): automation.validate_automation(single=True),
        cv.Required(CONF_OPEN_ENDSTOP): cv.use_id(binary_sensor.BinarySensor),
        cv.Required(CONF_OPEN_DURATION): cv.positive_time_period_milliseconds,
        cv.Required(CONF_CLOSE_ENDSTOP): cv.use_id(binary_sensor.BinarySensor),
        cv.Required(CONF_CLOSE_DURATION): cv.positive_time_period_milliseconds,

        cv.Optional(CONF_INTERVAL): cv.positive_time_period_milliseconds,
                    cv.Optional(CONF_MAX_DURATION): cv.positive_time_period_milliseconds,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await cover.new_cover(config)
    await cg.register_component(var, config)


    await automation.build_automation(
        var.get_tilt_trigger(), [], config[CONF_TILT_ACTION]
    )

    bin = await cg.get_variable(config[CONF_OPEN_ENDSTOP])
    cg.add(var.set_open_endstop(bin))

    cg.add(var.set_open_duration(config[CONF_OPEN_DURATION]))
    cg.add(var.set_close_duration(config[CONF_CLOSE_DURATION]))


    bin = await cg.get_variable(config[CONF_CLOSE_ENDSTOP])
    cg.add(var.set_close_endstop(bin))

    if CONF_MAX_DURATION in config:
        cg.add(var.set_max_duration(config[CONF_MAX_DURATION]))
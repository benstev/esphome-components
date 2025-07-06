from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover
from esphome import automation
from esphome.components import cc1101

from esphome.const import (
    CONF_CLOSE_DURATION,
    CONF_OPEN_DURATION,
)

REMOTE_CODE = "remote-code"
STORAGE_RECORD = "storage_record"
CONF_CC1101_ID = "cc1101-id"


somfy_ns = cg.esphome_ns.namespace("somfy")
SomfyCover = somfy_ns.class_("SomfyCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = cover.cover_schema(SomfyCover).extend(
    {
        cv.Required(CONF_OPEN_DURATION): cv.positive_time_period_milliseconds,
        cv.Required(CONF_CLOSE_DURATION): cv.positive_time_period_milliseconds,
        cv.Required(REMOTE_CODE):cv.positive_int,
        cv.Required(STORAGE_RECORD):cv.positive_int,
        cv.Required(CONF_CC1101_ID): cv.use_id(cc1101.CC1101),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await cover.new_cover(config)
    await cg.register_component(var, config)

    cg.add(var.set_open_duration(config[CONF_OPEN_DURATION]))
    cg.add(var.set_close_duration(config[CONF_CLOSE_DURATION]))

    cg.add(var.set_remote_code(config[REMOTE_CODE]))
    cg.add(var.set_storage_record(config[STORAGE_RECORD]))

    cc = await cg.get_variable(config[CONF_CC1101_ID])
    cg.add(var.set_cc1101(cc))


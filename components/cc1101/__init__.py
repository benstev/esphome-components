import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

somfy_remote_component_ns = cg.esphome_ns.namespace("cc1101")
CC1101 = somfy_remote_component_ns.class_("CC1101", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CC1101),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

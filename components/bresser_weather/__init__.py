from pathlib import Path

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, sensor, text_sensor
from esphome.const import (
    CONF_HUMIDITY,
    CONF_ID,
    CONF_TEMPERATURE,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_ILLUMINANCE,
    DEVICE_CLASS_PRECIPITATION,
    DEVICE_CLASS_SIGNAL_STRENGTH,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_WIND_SPEED,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_CELSIUS,
    UNIT_DEGREES,
    UNIT_LUX,
    UNIT_MILLIMETER,
    UNIT_METER_PER_SECOND,
    UNIT_PERCENT,
)

DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["binary_sensor", "json", "sensor", "text_sensor"]

CONF_WIND_GUST = "wind_gust"
CONF_WIND_SPEED = "wind_speed"
CONF_WIND_DIRECTION = "wind_direction"
CONF_RAIN = "rain"
CONF_UV = "uv"
CONF_ILLUMINANCE = "illuminance"
CONF_RSSI = "rssi"
CONF_BATTERY_LOW = "battery_low"
CONF_SENSOR_ID = "sensor_id"
CONF_FILTER_SENSOR_ID = "filter_sensor_id"
CONF_FREQUENCY_OFFSET = "frequency_offset"
CONF_LOG_SENSOR_ID = "log_sensor_id"
CONF_PINS = "pins"
CONF_SCK = "sck"
CONF_MISO = "miso"
CONF_MOSI = "mosi"
CONF_CS = "cs"
CONF_RESET = "reset"
CONF_IRQ = "irq"
CONF_DIO1 = "dio1"

UNIT_UV_INDEX = "UV index"
UNIT_DECIBEL_MILLIWATT = "dBm"

bresser_weather_ns = cg.esphome_ns.namespace("bresser_weather")
BresserWeatherComponent = bresser_weather_ns.class_(
    "BresserWeatherComponent", cg.Component
)

PIN_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SCK): cv.int_range(min=0, max=33),
        cv.Required(CONF_MISO): cv.int_range(min=0, max=39),
        cv.Required(CONF_MOSI): cv.int_range(min=0, max=33),
        cv.Required(CONF_CS): cv.int_range(min=0, max=33),
        cv.Required(CONF_RESET): cv.int_range(min=0, max=33),
        cv.Required(CONF_IRQ): cv.int_range(min=0, max=39),
        cv.Required(CONF_DIO1): cv.int_range(min=0, max=39),
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(BresserWeatherComponent),
        cv.Required(CONF_PINS): PIN_SCHEMA,
        cv.Optional(CONF_FILTER_SENSOR_ID): cv.hex_uint32_t,
        cv.Optional(CONF_LOG_SENSOR_ID, default=True): cv.boolean,
        cv.Optional(CONF_FREQUENCY_OFFSET, default=0.0): cv.float_range(
            min=-1.0, max=1.0
        ),
        cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_HUMIDITY): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_HUMIDITY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_WIND_GUST): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER_PER_SECOND,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_WIND_SPEED,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_WIND_SPEED): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER_PER_SECOND,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_WIND_SPEED,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_WIND_DIRECTION): sensor.sensor_schema(
            unit_of_measurement=UNIT_DEGREES,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_RAIN): sensor.sensor_schema(
            unit_of_measurement=UNIT_MILLIMETER,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_PRECIPITATION,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_UV): sensor.sensor_schema(
            unit_of_measurement=UNIT_UV_INDEX,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_ILLUMINANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_LUX,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_ILLUMINANCE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_RSSI): sensor.sensor_schema(
            unit_of_measurement=UNIT_DECIBEL_MILLIWATT,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_BATTERY_LOW): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_BATTERY,
        ),
        cv.Optional(CONF_SENSOR_ID): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_frequency_offset(config[CONF_FREQUENCY_OFFSET]))
    cg.add(var.set_log_sensor_id(config[CONF_LOG_SENSOR_ID]))

    if CONF_FILTER_SENSOR_ID in config:
        cg.add(var.set_filter_sensor_id(config[CONF_FILTER_SENSOR_ID]))

    sensor_setters = {
        CONF_TEMPERATURE: var.set_temperature_sensor,
        CONF_HUMIDITY: var.set_humidity_sensor,
        CONF_WIND_GUST: var.set_wind_gust_sensor,
        CONF_WIND_SPEED: var.set_wind_speed_sensor,
        CONF_WIND_DIRECTION: var.set_wind_direction_sensor,
        CONF_RAIN: var.set_rain_sensor,
        CONF_UV: var.set_uv_sensor,
        CONF_ILLUMINANCE: var.set_illuminance_sensor,
        CONF_RSSI: var.set_rssi_sensor,
    }
    for key, setter in sensor_setters.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(setter(sens))

    if CONF_BATTERY_LOW in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BATTERY_LOW])
        cg.add(var.set_battery_low_sensor(sens))

    if CONF_SENSOR_ID in config:
        sens = await text_sensor.new_text_sensor(config[CONF_SENSOR_ID])
        cg.add(var.set_sensor_id_text_sensor(sens))

    # Keep the receiver decoder in this ESPHome package. Its library.json also
    # declares RadioLib explicitly; this is required by ESPHome's ESP-IDF based
    # Arduino builder, which does not currently parse Arduino's `depends=` key.
    receiver_library = (
        Path(__file__).parent / "vendor" / "BresserWeatherSensorReceiver"
    ).resolve()
    cg.add_library(
        "BresserWeatherSensorReceiver",
        "0.41.1",
        receiver_library.as_uri(),
    )
    # ESPHome 2026 uses selective compilation for Arduino's bundled libraries.
    cg.add_library("Preferences", None)
    cg.add_library("SPI", None)

    # ESPHome's ESP-IDF based Arduino build intentionally uses the generic ESP32
    # variant, so PlatformIO's TTGO board macros/pins are not supplied. The
    # wiring is therefore defined explicitly in YAML and passed to the receiver.
    pins = config[CONF_PINS]
    for flag in (
        "-DARDUINO_TTGO_LoRa32_V1",
        "-DLORA_SPI_BUS=HSPI",
        f"-DLORA_SCK={pins[CONF_SCK]}",
        f"-DLORA_MISO={pins[CONF_MISO]}",
        f"-DLORA_MOSI={pins[CONF_MOSI]}",
        f"-DLORA_CS={pins[CONF_CS]}",
        f"-DLORA_RST={pins[CONF_RESET]}",
        f"-DLORA_IRQ={pins[CONF_IRQ]}",
        f"-DLORA_D1={pins[CONF_DIO1]}",
    ):
        cg.add_build_flag(flag)

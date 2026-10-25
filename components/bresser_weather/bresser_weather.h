#pragma once

#include <WeatherSensor.h>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome::bresser_weather {

class BresserWeatherComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_frequency_offset(float offset) { frequency_offset_ = offset; }
  void set_max_sensors(uint8_t value) { max_sensors_ = value; }
  void set_log_sensor_id(bool value) { log_sensor_id_ = value; }
  void set_filter_sensor_id(uint32_t id) {
    filter_sensor_id_ = id;
    filter_enabled_ = true;
  }

  void set_leakage_sensor_id(uint32_t id) {
    leakage_sensor_id_ = id;
    leakage_filter_enabled_ = true;
  }

  void set_temperature_sensor(sensor::Sensor *value) { temperature_sensor_ = value; }
  void set_humidity_sensor(sensor::Sensor *value) { humidity_sensor_ = value; }
  void set_wind_gust_sensor(sensor::Sensor *value) { wind_gust_sensor_ = value; }
  void set_wind_speed_sensor(sensor::Sensor *value) { wind_speed_sensor_ = value; }
  void set_wind_direction_sensor(sensor::Sensor *value) { wind_direction_sensor_ = value; }
  void set_rain_sensor(sensor::Sensor *value) { rain_sensor_ = value; }
  void set_uv_sensor(sensor::Sensor *value) { uv_sensor_ = value; }
  void set_illuminance_sensor(sensor::Sensor *value) { illuminance_sensor_ = value; }
  void set_rssi_sensor(sensor::Sensor *value) { rssi_sensor_ = value; }
  void set_battery_low_sensor(binary_sensor::BinarySensor *value) { battery_low_sensor_ = value; }
  void set_sensor_id_text_sensor(text_sensor::TextSensor *value) { sensor_id_sensor_ = value; }
  void set_leakage_alarm_sensor(binary_sensor::BinarySensor *value) { leakage_alarm_sensor_ = value; }

 protected:
  void publish_weather_(const WeatherSensor::sensor_t &data);

  WeatherSensor receiver_;
  float frequency_offset_{0.0f};
  uint8_t max_sensors_{4};
  uint32_t filter_sensor_id_{0};
  bool filter_enabled_{false};
  uint32_t leakage_sensor_id_{0};
  bool leakage_filter_enabled_{false};
  bool log_sensor_id_{true};

  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *humidity_sensor_{nullptr};
  sensor::Sensor *wind_gust_sensor_{nullptr};
  sensor::Sensor *wind_speed_sensor_{nullptr};
  sensor::Sensor *wind_direction_sensor_{nullptr};
  sensor::Sensor *rain_sensor_{nullptr};
  sensor::Sensor *uv_sensor_{nullptr};
  sensor::Sensor *illuminance_sensor_{nullptr};
  sensor::Sensor *rssi_sensor_{nullptr};
  binary_sensor::BinarySensor *battery_low_sensor_{nullptr};
  text_sensor::TextSensor *sensor_id_sensor_{nullptr};
  binary_sensor::BinarySensor *leakage_alarm_sensor_{nullptr};
};

}  // namespace esphome::bresser_weather

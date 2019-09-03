#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace ba01 {

class BA01Component : public PollingComponent, public uart::UARTDevice {
 public:
  float get_setup_priority() const override;

  void update() override;
  void dump_config() override;

  void set_tds1_sensor(sensor::Sensor *tds1_sensor) { tds1_sensor_ = tds1_sensor; }
  void set_temperature1_sensor(sensor::Sensor *temp1_sensor) { temp1_sensor_ = temp1_sensor; }
  void set_tds2_sensor(sensor::Sensor *tds2_sensor) { tds2_sensor_ = tds2_sensor; }
  void set_temperature2_sensor(sensor::Sensor *temp2_sensor) { temp2_sensor_ = temp2_sensor; }

 protected:
  bool ba01_write_command_(const uint8_t *command, uint8_t *response);

  sensor::Sensor *tds1_sensor_{nullptr};
  sensor::Sensor *temp1_sensor_{nullptr};
  sensor::Sensor *tds2_sensor_{nullptr};
  sensor::Sensor *temp2_sensor_{nullptr};
};

}  // namespace s8x
}  // namespace esphome

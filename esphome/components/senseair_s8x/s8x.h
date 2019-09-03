#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace s8x {

class S8XComponent : public PollingComponent, public uart::UARTDevice {
 public:
  float get_setup_priority() const override;

  void update() override;
  void dump_config() override;

  void set_co2_sensor(sensor::Sensor *co2_sensor) { co2_sensor_ = co2_sensor; }

 protected:
  bool s8x_write_command_(const uint8_t *command, uint8_t *response);

  sensor::Sensor *co2_sensor_{nullptr};
};

}  // namespace s8x
}  // namespace esphome

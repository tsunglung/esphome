#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace pzemac {
#define PZEM_AC_DEVICE_ADDRESS  0x01  // PZEM default address
#define CMD_RHR         0x03
#define CMD_RIR         0X04
#define CMD_WSR         0x06
#define CMD_CAL         0x41
#define CMD_REST        0x42
#define UPDATE_TIME     60

class PZEMAC : public PollingComponent, public uart::UARTDevice {
 public:
  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_frequency_sensor(sensor::Sensor *frequency_sensor) { frequency_sensor_ = frequency_sensor; }
  void set_powerfactor_sensor(sensor::Sensor *powerfactor_sensor) { powerfactor_sensor_ = powerfactor_sensor; }
  void set_energy_sensor(sensor::Sensor *energy_sensor) { energy_sensor_ = energy_sensor; }

  void loop() override;

  void update() override;

 protected:
  sensor::Sensor *voltage_sensor_;
  sensor::Sensor *current_sensor_;
  sensor::Sensor *power_sensor_;
  sensor::Sensor *frequency_sensor_;
  sensor::Sensor *powerfactor_sensor_;
  sensor::Sensor *energy_sensor_;

  void Send(uint8_t device_address, uint8_t function_code, uint16_t start_address, uint16_t register_count);
  bool resetEnergy();
  bool ReceiveReady();
  uint8_t ReceiveBuffer(uint8_t *buffer, uint8_t register_count);

private:
  uint8_t mb_address;
};

}  // namespace pzemac
}  // namespace esphome

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/xiaomi_ble/xiaomi_ble.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace xiaomi_miscale {

class XiaomiMiscale : public Component, public esp32_ble_tracker::ESPBTDeviceListener {
 public:
  void set_address(uint64_t address) { address_ = address; }
  void set_devicetimezone(int timezone) { timezone_ = timezone; }

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override {

    if (device.address_uint64() != this->address_)
      return false;

    auto res = xiaomi_ble::parse_xiaomi(device);
    if (!res.has_value())
      return false;

    if (res->weight.has_value() && this->weight_ != nullptr)
      this->weight_->publish_state(*res->weight);
    if (res->impedance.has_value() && this->impedance_ != nullptr)
      this->impedance_->publish_state(*res->impedance);
    if (res->datetime.has_value() && this->datetime_ != nullptr)
      this->datetime_->publish_state(*res->datetime);
    if (res->battery_level.has_value() && this->battery_level_ != nullptr)
      this->battery_level_->publish_state(*res->battery_level);
    return true;
  }

  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void set_weight(sensor::Sensor *weight) { weight_ = weight; }
  void set_impedance(sensor::Sensor *impedance) { impedance_ = impedance; }
  void set_datetime(sensor::Sensor *datetime) { datetime_ = datetime - 3600 * timezone_; }
  void set_battery_level(sensor::Sensor *battery_level) { battery_level_ = battery_level; }

 protected:
  uint64_t address_;
  int timezone_{0};
  sensor::Sensor *weight_{nullptr};
  sensor::Sensor *impedance_{nullptr};
  sensor::Sensor *datetime_{nullptr};
  sensor::Sensor *battery_level_{nullptr};
};

}  // namespace xiaomi_miscale
}  // namespace esphome

#endif

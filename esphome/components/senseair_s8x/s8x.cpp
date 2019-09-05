#include "s8x.h"
#include "esphome/core/log.h"

namespace esphome {
namespace s8x {

#define S8X_PREAMBLE_LENGTH 2
static const char *TAG = "s8x";
static const uint8_t S8X_REQUEST_LENGTH = 8;
static const uint8_t S8X_RESPONSE_LENGTH = 13;
static const uint8_t S8X_COMMAND_GET_PPM[] = {0xFE, 0x04, 0x00, 0x00, 0x00, 0x04, 0xE5, 0xC6};  //Command packet to read CO2 (see app note), CRC 0xC6E5

// ---=== Calc CRC16 ===---
uint16_t s8x_checksum(uint8_t *ptr, uint8_t length) {
  uint16_t crc = 0xFFFF;
  uint8_t i;
  //------------------------------
  while (length--) {
    crc ^= *ptr++;
    for (i = 0; i < 8; i++)
      if ((crc & 0x01) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else
        crc >>= 1;
  }
  return crc;
}

void S8XComponent::update() {
  uint8_t response[S8X_RESPONSE_LENGTH];
  if (!this->s8x_write_command_(S8X_COMMAND_GET_PPM, response)) {
    ESP_LOGW(TAG, "Reading data from SenseAir S8 failed!");
    this->status_set_warning();
    return;
  }

//ESP_LOGD(TAG, "SenseAir S8: %x %x %x %x %x %x %x", response[0], response[1], response[2], response[3], response[4], response[5], response[6]);
  if (response[0] != 0xFE || response[1] != 0x04) {
    ESP_LOGW(TAG, "Invalid preamble from SenseAir S8!");
    this->status_set_warning();
    return;
  }

  uint16_t checksum = s8x_checksum(response, S8X_RESPONSE_LENGTH - 2);
  if (((response[S8X_RESPONSE_LENGTH - 1] << 8) | response[S8X_RESPONSE_LENGTH - 2]) != checksum) {
    ESP_LOGW(TAG, "SenseAir S8 Checksum doesn't match: 0x%02X!=0x%02X", ((response[12] << 8) | response[11]), checksum);
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  const uint8_t length = response[2];
  const uint16_t status = (uint16_t(response[3]) << 8) | response[4];
  const uint16_t ppm = (uint16_t(response[S8X_PREAMBLE_LENGTH + length - 1]) << 8) | response[S8X_PREAMBLE_LENGTH + length];

  ESP_LOGD(TAG, "SenseAir S8 Received CO₂=%uppm, status = %x", ppm, status);
  if (this->co2_sensor_ != nullptr)
    this->co2_sensor_->publish_state(ppm);
}

bool S8XComponent::s8x_write_command_(const uint8_t *command, uint8_t *response) {
  this->flush();
  this->write_array(command, S8X_REQUEST_LENGTH);

  if (response == nullptr)
    return true;

  bool ret = this->read_array(response, S8X_RESPONSE_LENGTH);
  this->flush();
  return ret;
}
float S8XComponent::get_setup_priority() const { return setup_priority::DATA; }
void S8XComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "SenseAir S8:");
  LOG_SENSOR("  ", "CO₂", this->co2_sensor_);
}

}  // namespace s8x
}  // namespace esphome

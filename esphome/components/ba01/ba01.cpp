#include "ba01.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ba01 {

static const char *TAG = "ba01";
static const uint8_t BA01_REQUEST_LENGTH = 6;
static const uint8_t BA01_RESPONSE_LENGTH = 12;
static const uint8_t BA01_COMMAND_GET_PPM[] = {0xA0, 0x00, 0x00, 0x00, 0x00, 0xA0};  //Command packet to read TDS (see app note), CRC 0xa0

// ---=== Calc CRC8 ===---
uint8_t ba01_checksum(uint8_t *ptr, uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < length; i++) {
    sum += ptr[i];
  }
  return sum;
}

void BA01Component::update() {
  uint8_t response[BA01_RESPONSE_LENGTH];
  if (!this->ba01_write_command_(BA01_COMMAND_GET_PPM, response)) {
    ESP_LOGW(TAG, "Reading data from BA01 failed!");
    this->status_set_warning();
    return;
  }

//ESP_LOGD(TAG, "BA01: %x %x %x %x %x %x", response[0], response[1], response[2], response[3], response[4], response[5]);
//ESP_LOGD(TAG, "BA01: %x %x %x %x %x %x", response[6], response[7], response[8], response[9], response[10], response[11]);
  if (response[0] == 0xAC) {
    if (response[1] == 0x01)
      ESP_LOGW(TAG, "BA01 %x: command error %x!", response[0], response[1]);
    if (response[1] == 0x02)
      ESP_LOGW(TAG, "BA01 %x: busy error %x!", response[0], response[1]);
    if (response[1] == 0x03)
      ESP_LOGW(TAG, "BA01 %x: crc failed error %x!", response[0], response[1]);
    if (response[1] == 0x04)
      ESP_LOGW(TAG, "BA01 %x: over temperature error %x!", response[0], response[1]);
    this->status_set_warning();
    return;  
  }
  if ((response[0] != 0xAA) && (response[0] != 0xAB)) {
    ESP_LOGW(TAG, "Invalid preamble from BA01!");
    this->status_set_warning();
    return;
  }

  uint8_t checksum = ba01_checksum(response, BA01_RESPONSE_LENGTH/2 - 1);
  if (response[5] != checksum) {
    ESP_LOGW(TAG, "BA01 Checksum doesn't match: 0x%01X!=0x%01X", response[5], checksum);
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  if (response[0] == 0xAA) {
    const uint16_t ppm1 = (uint16_t(response[1]) << 8) | response[2];
    const uint16_t ppm2 = (uint16_t(response[3]) << 8) | response[4];

    ESP_LOGD(TAG, "BA01 Received TDS1=%uppm, TDS2=%uppm", ppm1, ppm2);
    if (this->tds1_sensor_ != nullptr)
      this->tds1_sensor_->publish_state(ppm1);
    if (this->tds2_sensor_ != nullptr)
      this->tds2_sensor_->publish_state(ppm2);
  }

  if ((response[6] != 0xAA) && (response[6] != 0xAB)) {
    ESP_LOGW(TAG, "Invalid preamble from BA01!");
    this->status_set_warning();
    return;
  }

  checksum = ba01_checksum(&response[6], BA01_RESPONSE_LENGTH/2 - 1);
  if (response[11] != checksum) {
    ESP_LOGW(TAG, "BA01 Checksum doesn't match: 0x%01X!=0x%01X", response[11], checksum);
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  if (response[6] == 0xAB) {
    const float temp1 = ((uint16_t(response[7]) << 8) | response[8])/100.0;
    const float temp2 = ((uint16_t(response[9]) << 8) | response[10])/100.0;

    ESP_LOGD(TAG, "BA01 Received Temp1=%.2f°C, Temp2=%.2f°C", temp1, temp2);
    if (this->temp1_sensor_ != nullptr)
      this->temp1_sensor_->publish_state(temp1);
    if (this->temp2_sensor_ != nullptr)
      this->temp2_sensor_->publish_state(temp2);
  }
}

bool BA01Component::ba01_write_command_(const uint8_t *command, uint8_t *response) {
  this->flush();
  this->write_array(command, BA01_REQUEST_LENGTH);

  if (response == nullptr)
    return true;

  // wait for data
  delay(1000);
  bool ret = this->read_array(response, BA01_RESPONSE_LENGTH);
  this->flush();
  return ret;
}
float BA01Component::get_setup_priority() const { return setup_priority::DATA; }
void BA01Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BA01:");
  LOG_SENSOR("  ", "TDS1", this->tds1_sensor_);
  LOG_SENSOR("  ", "TDS2", this->tds2_sensor_);
  LOG_SENSOR("  ", "Temperature1", this->temp1_sensor_);
  LOG_SENSOR("  ", "Temperature2", this->temp2_sensor_);
}

}  // namespace ba01
}  // namespace esphome

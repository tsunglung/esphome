#include "pzemac.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pzemac {

static const char *TAG = "pzemac";

// ---=== Calc CRC16 ===---
uint16_t crc_16(uint8_t *ptr, uint8_t length) {
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

bool PZEMAC::ReceiveReady()
{
  return (this->available() > 4);
}

uint8_t PZEMAC::ReceiveBuffer(uint8_t *buffer, uint8_t register_count) {
  uint8_t len = 0;
  uint32_t last = millis();

  /* reference to Sonoff-Tasmota, https://github.com/arendst/Sonoff-Tasmota/blob/development/lib/TasmotaModbus-1.1.0/src/TasmotaModbus.cpp */
  while ((this->available() > 0) && (len < (register_count *2) + 5) && (millis() - last < 10)) {
    uint8_t data = (uint8_t)this->read();
    if (!len) {                  // Skip leading data as provided by hardware serial
      if (mb_address == data) {
        buffer[len++] = data;
      }
    } else {
      buffer[len++] = data;
      if (3 == len) {
        if (buffer[1] & 0x80) {  // 01 84 02 f2 f1
          return buffer[2];      // 1 = Illegal Function, 2 = Illegal Address, 3 = Illegal Data, 4 = Slave Error
        }
      }
    }
    last = millis();
  }

  if (len < 7) { return 7; }               // 7 = Not enough data
  if (len != buffer[2] + 5) { return 8; }  // 8 = Unexpected result

  uint16_t crc = (buffer[len -1] << 8) | buffer[len -2];
  if (crc_16(buffer, len -2) != crc) { return 9; }  // 9 = crc error

  return 0;                                // 0 = No error
}

void PZEMAC::loop() {

  static uint8_t send_retry = 0;
  // wait for data
  delay(UPDATE_TIME);

  bool data_ready = this->ReceiveReady();
  if (data_ready) {
    uint8_t buffer[26];

    uint8_t error = this->ReceiveBuffer(buffer, 10);
    if (error) {
        ESP_LOGD(TAG, "PzemAc response error %d", error);
    } else {
        /* reference to Sonoff-Tasmota, https://github.com/arendst/Sonoff-Tasmota/blob/development/sonoff/xnrg_05_pzem_ac.ino */
        unsigned long energy_kWhtoday;
        float energy, energy_start, energy_voltage, energy_current, energy_active_power, energy_frequency, energy_power_factor;
        //  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
        // 01 04 14 08 D1 00 6C 00 00 00 F4 00 00 00 26 00 00 01 F4 00 64 00 00 51 34
        // Id Cc Sz Volt- Current---- Power------ Energy----- Frequ PFact Alarm Crc--
        energy_voltage = (float)((buffer[3] << 8) + buffer[4]) / 10.0;                                                  // 6553.0 V
        energy_current = (float)((buffer[7] << 24) + (buffer[8] << 16) + (buffer[5] << 8) + buffer[6]) / 1000.0;        // 4294967.000 A
        energy_active_power = (float)((buffer[11] << 24) + (buffer[12] << 16) + (buffer[9] << 8) + buffer[10]) / 10.0;  // 429496729.0 W
        energy_frequency = (float)((buffer[17] << 8) + buffer[18]) / 10.0;                                              // 50.0 Hz
        energy_power_factor = (float)((buffer[19] << 8) + buffer[20]) / 100.0;                                          // 1.00
        energy = (float)((buffer[15] << 24) + (buffer[16] << 16) + (buffer[13] << 8) + buffer[14]);               // 4294967295 Wh

        ESP_LOGD(TAG, "V=%.1f I=%.3f P=%.1f F=%.1f PF=%.2f Wh=%.0f CRC=%x calCRC=%x", 
                       energy_voltage, energy_current, energy_active_power, energy_frequency, energy_power_factor, energy,
                       ((buffer[24] << 8) | buffer[23]), crc_16(buffer, 23));
        if (this->voltage_sensor_ != nullptr)
          this->voltage_sensor_->publish_state(energy_voltage);
        ESP_LOGVV(TAG, "Got Voltage %.1f V", energy_voltage);
        if (this->current_sensor_ != nullptr)
          this->current_sensor_->publish_state(energy_current);
        ESP_LOGVV(TAG, "Got Current %.3f A", energy_current);
        if (this->power_sensor_ != nullptr)
          this->power_sensor_->publish_state(energy_active_power);
        ESP_LOGVV(TAG, "Got Power %.1f W", energy_active_power);
        if (this->frequency_sensor_ != nullptr)
          this->frequency_sensor_->publish_state(energy_frequency);
        ESP_LOGVV(TAG, "Got Frequency %.1f Hz", energy_frequency);
        if (this->powerfactor_sensor_ != nullptr)
          this->powerfactor_sensor_->publish_state(energy_power_factor);
        ESP_LOGVV(TAG, "Got Power Factor %.2f", energy_power_factor);
        if (this->energy_sensor_ != nullptr)
          this->energy_sensor_->publish_state(energy);
        ESP_LOGVV(TAG, "Got Energy %.0f Wh", energy);
/*
        if (!energy_start || (energy < energy_start)) { energy_start = energy; }  // Init after restart and handle roll-over if any
        if (energy != energy_start) {
          energy_kWhtoday += (unsigned long)((energy - energy_start) * 100);
          energy_start = energy;
        }
        EnergyUpdateToday();
*/
     }
  }

  if (0 == send_retry || data_ready) {
    send_retry = 5;
    delay(5);
  }
  else {
    send_retry--;
  }
}

void PZEMAC::update() { this->Send(PZEM_AC_DEVICE_ADDRESS, CMD_RIR, 0, 10); }

bool PZEMAC::resetEnergy() { 
  uint8_t buffer[] = {0x00, CMD_REST, 0x00, 0x00};
  uint8_t reply[5];
  buffer[0] = PZEM_AC_DEVICE_ADDRESS;

  uint16_t crc = crc_16(buffer, 2);
  buffer[2] = (uint8_t)(crc);
  buffer[3] = (uint8_t)(crc >> 8);
  this->write_array(buffer, 4);

  uint16_t length = ReceiveBuffer(reply, 5);

  if(length == 0 || length == 5){
    return false;
  }

  return true;
}

void PZEMAC::Send(uint8_t device_address, uint8_t function_code, uint16_t start_address, uint16_t register_count)
{
  uint8_t frame[8];

  mb_address = device_address;  // Save address for receipt check

  frame[0] = mb_address;        // 0xFE default device address or dedicated like 0x01
  frame[1] = function_code;
  frame[2] = (uint8_t)(start_address >> 8);
  frame[3] = (uint8_t)(start_address);
  frame[4] = (uint8_t)(register_count >> 8);
  frame[5] = (uint8_t)(register_count);
  uint16_t crc = crc_16(frame, 6);
  frame[6] = (uint8_t)(crc);
  frame[7] = (uint8_t)(crc >> 8);

  this->flush();
  this->write_array(frame, sizeof(frame));
}

}  // namespace pzemac
}  // namespace esphome

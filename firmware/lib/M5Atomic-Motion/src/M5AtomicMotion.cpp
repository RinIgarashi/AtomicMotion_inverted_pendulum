#include "M5AtomicMotion.h"

bool M5AtomicMotion::begin(m5::I2C_Class* i2c, uint8_t addr, uint8_t sda, uint8_t scl, long freq) {
  _i2c = i2c;
  _addr = addr;
  _freq = freq;

  if (!_i2c->begin((i2c_port_t)0, sda, scl)) {
    return false;
  }

  return _i2c->scanID(_addr);
}

uint8_t M5AtomicMotion::setServoAngle(uint8_t servo_ch, uint8_t angle) {
  uint8_t reg = servo_ch;
  if (reg > 3) return 0;
  _i2c->writeRegister8(_addr, reg, angle, _freq);
  return 0;
}

uint8_t M5AtomicMotion::setServoPulse(uint8_t servo_ch,
                                      uint16_t width)  // 0x10        ->16
{
  uint8_t reg = 2 * servo_ch + 16;
  uint8_t data[2] = {(uint8_t)(width >> 8), (uint8_t)(width & 0xFF)};
  if (reg % 2 == 1 || reg > 32) return 1;

  _i2c->writeRegister(_addr, reg, data, 2, _freq);
  return 0;
}

uint8_t M5AtomicMotion::setMotorSpeed(uint8_t motor_ch,
                                      int8_t speed)  // 0x10 ->16
{
  if (motor_ch > 1) return 1;
  uint8_t reg = motor_ch + 32;
  _i2c->writeRegister8(_addr, reg, (uint8_t)speed, _freq);
  return 0;
}

uint8_t M5AtomicMotion::getServoAngle(uint8_t servo_ch) {
  uint8_t reg = servo_ch;
  return _i2c->readRegister8(_addr, reg, _freq);
}

uint16_t M5AtomicMotion::getServoPulse(uint8_t servo_ch) {
  uint8_t data[2] = {0};
  uint8_t reg = 2 * servo_ch | 0x10;
  _i2c->readRegister(_addr, reg, data, 2, _freq);
  return (data[0] << 8) + data[1];
}

int8_t M5AtomicMotion::getMotorSpeed(uint8_t motor_ch) {
  if (motor_ch > 1) return 1;
  uint8_t reg = motor_ch | 0x20;
  return (int8_t)_i2c->readRegister8(_addr, reg, _freq);
}

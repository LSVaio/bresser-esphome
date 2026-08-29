#pragma once

// Derived from RadioLib's EspHal.
// Copyright (c) 2018 Jan Gromes. Licensed under the MIT License.
// See ../RadioLib-license.txt.

#if defined(BRESSER_ESPHOME_IDF)

#include <RadioLib.h>

#include "driver/spi_master.h"

// Native ESP-IDF RadioLib HAL. This is based on RadioLib's MIT-licensed
// EspHal implementation and uses the public spi_master driver on every ESP32
// variant supported by ESP-IDF.
class BresserEspHal : public RadioLibHal {
 public:
  BresserEspHal(int8_t sck, int8_t miso, int8_t mosi,
                spi_host_device_t host = SPI2_HOST, uint32_t clock_hz = 2000000);
  ~BresserEspHal() override;

  void pinMode(uint32_t pin, uint32_t mode) override;
  void digitalWrite(uint32_t pin, uint32_t value) override;
  uint32_t digitalRead(uint32_t pin) override;
  void attachInterrupt(uint32_t interrupt_num, void (*callback)(void), uint32_t mode) override;
  void detachInterrupt(uint32_t interrupt_num) override;
  void delay(RadioLibTime_t milliseconds) override;
  void delayMicroseconds(RadioLibTime_t microseconds) override;
  RadioLibTime_t millis() override;
  RadioLibTime_t micros() override;
  long pulseIn(uint32_t pin, uint32_t state, RadioLibTime_t timeout) override;
  void spiBegin() override;
  void spiBeginTransaction() override;
  void spiTransfer(uint8_t *out, size_t length, uint8_t *in) override;
  void spiEndTransaction() override;
  void spiEnd() override;
  void init() override;
  void term() override;
  void yield() override;
  void pullUpDown(uint32_t pin, bool enable, bool up) override;

 protected:
  int8_t sck_;
  int8_t miso_;
  int8_t mosi_;
  spi_host_device_t host_;
  uint32_t clock_hz_;
  spi_device_handle_t device_{nullptr};
  bool owns_bus_{false};
  bool device_added_{false};
  bool initialized_{false};
};

#endif

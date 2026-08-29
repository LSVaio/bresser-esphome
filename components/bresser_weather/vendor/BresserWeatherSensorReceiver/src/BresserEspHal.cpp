#include "BresserEspHal.h"

// Derived from RadioLib's EspHal.
// Copyright (c) 2018 Jan Gromes. Licensed under the MIT License.
// See ../RadioLib-license.txt.

#if defined(BRESSER_ESPHOME_IDF)

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"

static const char *const BRESSER_HAL_TAG = "bresser_radio_hal";

BresserEspHal::BresserEspHal(int8_t sck, int8_t miso, int8_t mosi,
                             spi_host_device_t host, uint32_t clock_hz)
    : RadioLibHal(GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, 0, 1,
                  GPIO_INTR_POSEDGE, GPIO_INTR_NEGEDGE),
      sck_(sck), miso_(miso), mosi_(mosi), host_(host), clock_hz_(clock_hz) {}

BresserEspHal::~BresserEspHal() { this->term(); }

void BresserEspHal::pinMode(uint32_t pin, uint32_t mode) {
  if (pin == RADIOLIB_NC) return;
  gpio_config_t config{};
  config.pin_bit_mask = 1ULL << pin;
  config.mode = mode == GPIO_MODE_OUTPUT ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.intr_type = GPIO_INTR_DISABLE;
  gpio_config(&config);
}

void BresserEspHal::digitalWrite(uint32_t pin, uint32_t value) {
  if (pin != RADIOLIB_NC) gpio_set_level(static_cast<gpio_num_t>(pin), value);
}

uint32_t BresserEspHal::digitalRead(uint32_t pin) {
  return pin == RADIOLIB_NC ? 0 : gpio_get_level(static_cast<gpio_num_t>(pin));
}

void BresserEspHal::attachInterrupt(uint32_t interrupt_num, void (*callback)(void), uint32_t mode) {
  if (interrupt_num == RADIOLIB_NC) return;
  gpio_set_intr_type(static_cast<gpio_num_t>(interrupt_num), static_cast<gpio_int_type_t>(mode));
  gpio_isr_handler_add(static_cast<gpio_num_t>(interrupt_num),
                       reinterpret_cast<gpio_isr_t>(callback), nullptr);
}

void BresserEspHal::detachInterrupt(uint32_t interrupt_num) {
  if (interrupt_num == RADIOLIB_NC) return;
  gpio_isr_handler_remove(static_cast<gpio_num_t>(interrupt_num));
  gpio_set_intr_type(static_cast<gpio_num_t>(interrupt_num), GPIO_INTR_DISABLE);
}

void BresserEspHal::delay(RadioLibTime_t milliseconds) {
  if (milliseconds == 0) return;
  const int64_t target = esp_timer_get_time() + static_cast<int64_t>(milliseconds) * 1000;
  const TickType_t ticks = pdMS_TO_TICKS(milliseconds);
  if (ticks > 1) vTaskDelay(ticks - 1);
  const int64_t remaining = target - esp_timer_get_time();
  if (remaining > 0) esp_rom_delay_us(static_cast<uint32_t>(remaining));
}

void BresserEspHal::delayMicroseconds(RadioLibTime_t microseconds) {
  esp_rom_delay_us(microseconds);
}

RadioLibTime_t BresserEspHal::millis() {
  return static_cast<RadioLibTime_t>(esp_timer_get_time() / 1000ULL);
}

RadioLibTime_t BresserEspHal::micros() {
  return static_cast<RadioLibTime_t>(esp_timer_get_time());
}

long BresserEspHal::pulseIn(uint32_t pin, uint32_t state, RadioLibTime_t timeout) {
  if (pin == RADIOLIB_NC) return 0;
  const RadioLibTime_t wait_start = this->micros();
  while (this->digitalRead(pin) != state) {
    if (this->micros() - wait_start >= timeout) return 0;
  }
  const RadioLibTime_t pulse_start = this->micros();
  while (this->digitalRead(pin) == state) {
    if (this->micros() - pulse_start >= timeout) return 0;
  }
  return static_cast<long>(this->micros() - pulse_start);
}

void BresserEspHal::spiBegin() {
  spi_bus_config_t bus{};
  bus.mosi_io_num = this->mosi_;
  bus.miso_io_num = this->miso_;
  bus.sclk_io_num = this->sck_;
  bus.quadwp_io_num = -1;
  bus.quadhd_io_num = -1;
  bus.data4_io_num = -1;
  bus.data5_io_num = -1;
  bus.data6_io_num = -1;
  bus.data7_io_num = -1;
  bus.max_transfer_sz = SOC_SPI_MAXIMUM_BUFFER_SIZE;
  bus.isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO;

  esp_err_t error = spi_bus_initialize(this->host_, &bus, SPI_DMA_CH_AUTO);
  if (error == ESP_OK) {
    this->owns_bus_ = true;
  } else if (error != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(BRESSER_HAL_TAG, "SPI bus initialization failed: %s", esp_err_to_name(error));
    return;
  }

  spi_device_interface_config_t device{};
  device.mode = 0;
  device.clock_speed_hz = static_cast<int>(this->clock_hz_);
  device.spics_io_num = -1;
  device.queue_size = 1;
  error = spi_bus_add_device(this->host_, &device, &this->device_);
  if (error != ESP_OK) {
    ESP_LOGE(BRESSER_HAL_TAG, "Adding SPI device failed: %s", esp_err_to_name(error));
    return;
  }
  this->device_added_ = true;
}

void BresserEspHal::spiBeginTransaction() {
  if (this->device_ != nullptr) spi_device_acquire_bus(this->device_, portMAX_DELAY);
}

void BresserEspHal::spiTransfer(uint8_t *out, size_t length, uint8_t *in) {
  if (this->device_ == nullptr || length == 0) return;
  spi_transaction_t transaction{};
  transaction.length = length * 8;
  transaction.tx_buffer = out;
  transaction.rx_buffer = in;
  const esp_err_t error = spi_device_polling_transmit(this->device_, &transaction);
  if (error != ESP_OK) {
    ESP_LOGE(BRESSER_HAL_TAG, "SPI transfer failed: %s", esp_err_to_name(error));
  }
}

void BresserEspHal::spiEndTransaction() {
  if (this->device_ != nullptr) spi_device_release_bus(this->device_);
}

void BresserEspHal::spiEnd() {
  if (this->device_added_ && this->device_ != nullptr) {
    spi_bus_remove_device(this->device_);
    this->device_ = nullptr;
    this->device_added_ = false;
  }
  if (this->owns_bus_) {
    spi_bus_free(this->host_);
    this->owns_bus_ = false;
  }
}

void BresserEspHal::init() {
  if (this->initialized_) return;
  this->spiBegin();
  const esp_err_t error = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  if (error != ESP_OK && error != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(BRESSER_HAL_TAG, "GPIO ISR service initialization failed: %s", esp_err_to_name(error));
  }
  this->initialized_ = true;
}

void BresserEspHal::term() {
  if (!this->initialized_) return;
  this->spiEnd();
  this->initialized_ = false;
}

void BresserEspHal::yield() { taskYIELD(); }

void BresserEspHal::pullUpDown(uint32_t pin, bool enable, bool up) {
  if (pin == RADIOLIB_NC) return;
  gpio_set_pull_mode(static_cast<gpio_num_t>(pin),
                     enable ? (up ? GPIO_PULLUP_ONLY : GPIO_PULLDOWN_ONLY) : GPIO_FLOATING);
}

#endif

#pragma once

#if defined(WAVESHARE_ESP32P4_ETH_SX1262)

// LoRa SX1262 pins — connected on header P1 via GP-SPI3 (GPIO Matrix)
#define P_LORA_MOSI    17
#define P_LORA_MISO    16
#define P_LORA_SCLK    18
#define P_LORA_NSS     19   // CS
#define P_LORA_DIO_1   14   // IRQ
#define P_LORA_BUSY    15
#define P_LORA_RESET   22
#define P_LORA_DIO_0   -1   // NC

// Ethernet RMII pins (IP101GRI PHY, native ESP32-P4 EMAC)
// PHY provides 50 MHz REFCLK on GPIO50 (clock input mode)
#define ETH_PHY_ADDR   0    // AD0 and AD3 both grounded
#define ETH_MDC_GPIO   31
#define ETH_MDIO_GPIO  52
#define ETH_PHY_RST_GPIO 51
// RMII signals (handled by EMAC hardware, not user-configurable):
//   RMII_CLK = GPIO50 (input from PHY)
//   TXEN     = GPIO49, TXD0 = GPIO34, TXD1 = GPIO35
//   CRS_DV   = GPIO28, RXD0 = GPIO29, RXD1 = GPIO30

// No user button defined on this board's schematic
// Uncomment and set to correct GPIO if a USER_BTN is identified:
// #define PIN_USER_BTN_ANA <gpio>  // TODO: verify from schematic

#include <Arduino.h>
#include "helpers/ESP32Board.h"

class ESP32P4EthBoard : public ESP32Board {
public:
  void begin();
  void startNetwork();
  void startEthernet();
  void reconfigureEthernet(uint32_t ip, uint32_t gw, uint32_t subnet, uint32_t dns1 = 0) override;

  // Deep sleep: ESP32-P4 (RISC-V) sleep API differs from ESP32-S3.
  // TODO: implement proper LP-core / light sleep when arduino-esp32 P4 support matures.
  void enterDeepSleep(uint32_t secs, int pin_wake_btn) {
    if (secs > 0) {
      esp_sleep_enable_timer_wakeup((uint64_t)secs * 1000000ULL);
    }
    esp_deep_sleep_start();
  }

  uint16_t getBattMilliVolts() override { return 0; }

  const char* getManufacturerName() const override { return "Waveshare ESP32-P4-ETH"; }
};

#endif  // WAVESHARE_ESP32P4_ETH_SX1262

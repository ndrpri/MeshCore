#include <Arduino.h>
#include "target.h"

ESP32P4EthBoard board;

#ifdef DISPLAY_CLASS
  DISPLAY_CLASS display;
#endif

#ifdef PIN_USER_BTN_ANA
  #include <helpers/ui/MomentaryButton.h>
  MomentaryButton user_btn(PIN_USER_BTN_ANA, 1000, true);
  MomentaryButton analog_btn(PIN_USER_BTN_ANA, 1000, true);
#endif

// Waveshare LR20/30 module: SX1262 chip + STM32 bridge + external RF switch
// RXEN/TXEN are controlled by the STM32 on the module, NOT wired to the MCU.
// All SPI pins routed through GPIO Matrix (GP-SPI, default FSPI host).
static SPIClass spi;

RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, spi);

WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);

EnvironmentSensorManager sensors;

bool radio_init() {
  fallback_clock.begin();
  rtc_clock.begin(Wire);
  return radio.std_init(&spi);
}

uint32_t radio_get_rng_seed() {
  return radio.random(0x7FFFFFFF);
}

void radio_set_params(float freq, float bw, uint8_t sf, uint8_t cr) {
  radio.setFrequency(freq);
  radio.setSpreadingFactor(sf);
  radio.setBandwidth(bw);
  radio.setCodingRate(cr);
}

void radio_set_tx_power(int8_t dbm) {
  radio.setOutputPower(dbm);
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);
}

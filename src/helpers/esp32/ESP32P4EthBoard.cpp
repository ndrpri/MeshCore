#if defined(WAVESHARE_ESP32P4_ETH_SX1262)

#include <Arduino.h>
#include <ETH.h>
#include <esp_task_wdt.h>
#include "ESP32P4EthBoard.h"

// ETH_CLOCK_GPIO50_IN: clock input from IP101GRI PHY on GPIO50.
// This constant is defined in arduino-esp32 3.x for ESP32-P4 EMAC.
// TODO: verify exact enum name when arduino-esp32 ESP32-P4 target is stable.
#ifndef ETH_CLOCK_GPIO50_IN
  #define ETH_CLOCK_GPIO50_IN EMAC_CLK_EXT_IN
#endif

String eth_local_ip;  // global, accessible via extern from ESP32Board.cpp

void ESP32P4EthBoard::begin() {
  ESP32Board::begin();

#ifdef USE_ETHERNET
  WiFi.mode(WIFI_OFF);
#endif

  startNetwork();
}

void ESP32P4EthBoard::startNetwork() {
#ifdef USE_ETHERNET
  startEthernet();
#endif
}

void ESP32P4EthBoard::startEthernet() {
  // Native RMII Ethernet via ESP32-P4 internal EMAC + IP101GRI PHY.
  // ETHClass2 is NOT used here. This calls the arduino-esp32 3.x ETH API directly.
  ETH.begin(ETH_PHY_IP101, ETH_PHY_ADDR, ETH_MDC_GPIO, ETH_MDIO_GPIO,
            ETH_PHY_RST_GPIO, ETH_CLOCK_GPIO50_IN);
  delay(100);

  // Wait for Ethernet link
  unsigned long t0 = millis();
  while (!ETH.linkUp() && millis() - t0 < 5000) {
    esp_task_wdt_reset();
    delay(100);
  }

  // Wait for DHCP IP assignment
  t0 = millis();
  while (ETH.localIP() == IPAddress(0, 0, 0, 0) && millis() - t0 < 5000) {
    esp_task_wdt_reset();
    delay(100);
  }

  // Apply static IP if DHCP timed out
  if (ETH.localIP() == IPAddress(0, 0, 0, 0)) {
#ifdef ETH_STATIC_IP
    Serial.println("DHCP timeout, using static IP from build flags");
    ETH.config(IPAddress(ETH_STATIC_IP), IPAddress(ETH_GATEWAY),
               IPAddress(ETH_SUBNET), IPAddress(ETH_DNS));
#else
    Serial.println("DHCP timeout, using fallback IP");
    ETH.config(IPAddress(192, 168, 4, 2), IPAddress(192, 168, 4, 1),
               IPAddress(255, 255, 255, 0));
#endif
  }

  eth_local_ip = ETH.localIP().toString();

  uint8_t mac[6];
  ETH.macAddress(mac);
  Serial.printf("ETH MAC %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("ETH IP "); Serial.println(ETH.localIP());
  Serial.println(ETH.linkUp() ? "ETH LINK UP" : "ETH LINK DOWN");
}

void ESP32P4EthBoard::reconfigureEthernet(uint32_t ip, uint32_t gw, uint32_t subnet, uint32_t dns1) {
  if (ip != 0) {
    uint8_t b1 = (ip >> 24) & 0xFF;
    uint8_t b2 = (ip >> 16) & 0xFF;
    uint8_t b3 = (ip >> 8) & 0xFF;
    uint8_t b4 = ip & 0xFF;

    uint8_t gw1 = (gw >> 24) & 0xFF;
    uint8_t gw2 = (gw >> 16) & 0xFF;
    uint8_t gw3 = (gw >> 8) & 0xFF;
    uint8_t gw4 = gw & 0xFF;

    uint8_t sub1 = (subnet >> 24) & 0xFF;
    uint8_t sub2 = (subnet >> 16) & 0xFF;
    uint8_t sub3 = (subnet >> 8) & 0xFF;
    uint8_t sub4 = subnet & 0xFF;

    uint8_t dns_1 = (dns1 >> 24) & 0xFF;
    uint8_t dns_2 = (dns1 >> 16) & 0xFF;
    uint8_t dns_3 = (dns1 >> 8) & 0xFF;
    uint8_t dns_4 = dns1 & 0xFF;

    bool ok = ETH.config(
      IPAddress(b1, b2, b3, b4),
      IPAddress(gw1, gw2, gw3, gw4),
      IPAddress(sub1, sub2, sub3, sub4),
      IPAddress(dns_1, dns_2, dns_3, dns_4)
    );
    if (ok) {
      Serial.printf("ETH reconfigured to %d.%d.%d.%d\n", b1, b2, b3, b4);
    } else {
      Serial.println("ETH reconfigure failed");
    }
    eth_local_ip = ETH.localIP().toString();
  }
}

#endif  // WAVESHARE_ESP32P4_ETH_SX1262

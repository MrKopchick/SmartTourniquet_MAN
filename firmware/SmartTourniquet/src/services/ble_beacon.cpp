#include "ble_beacon.h"
#include <bluefruit.h>
#include "../../config.h"

// BLE advertising interval expressed in 0.625 ms units.
static constexpr uint16_t kAdvUnits = (uint32_t)BLE_ADV_INTERVAL_MS * 8 / 5;

bool BleBeacon::begin() {
  Bluefruit.begin();
  Bluefruit.autoConnLed(false);          // Disable diagnostic LEDs for light discipline
  Bluefruit.setTxPower(BLE_TX_POWER_DBM);
  Bluefruit.setName(BLE_DEVICE_NAME);
  Bluefruit.Advertising.restartOnDisconnect(false);
  return true;
}

void BleBeacon::update(const BeaconPacket::Payload& p) {
  uint8_t buf[BeaconPacket::LENGTH];
  BeaconPacket::encode(p, buf);

  // Payload: Flags (3) + Manufacturer Data (2 + 12) + Name (2 + 3) = 22 of 31 max bytes.
  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buf, sizeof(buf));
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED);
  Bluefruit.Advertising.setInterval(kAdvUnits, kAdvUnits);
  Bluefruit.Advertising.start(0);  // 0 = advertise indefinitely
  _started = true;

  // TODO: Implement adaptive intervals based on triage level (e.g., 2000 ms SAFE, 500 ms CRITICAL).
}

void BleBeacon::stop() {
  if (!_started) return;
  Bluefruit.Advertising.stop();
  _started = false;
}

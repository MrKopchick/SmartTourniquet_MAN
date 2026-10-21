#include "ble_beacon.h"
#include <bluefruit.h>
#include "../../config.h"

uint16_t BleBeacon::intervalUnitsFor(TriageLevel level) {
  uint32_t ms = BLE_ADV_SAFE_MS;
  switch (level) {
    case TriageLevel::CRITICAL:
      ms = BLE_ADV_CRITICAL_MS;
      break;
    case TriageLevel::ATTENTION:
      ms = BLE_ADV_ATTENTION_MS;
      break;
    case TriageLevel::SAFE:
    default:
      ms = BLE_ADV_SAFE_MS;
      break;
  }
  // BLE advertising interval is specified in 0.625 ms units (ms * 8 / 5).
  return static_cast<uint16_t>((ms * 8UL) / 5UL);
}

bool BleBeacon::begin() {
  // 0 peripheral and 0 central connections: pure beacon role saves SoftDevice RAM.
  Bluefruit.begin(0, 0);
  Bluefruit.autoConnLed(false);
  Bluefruit.setTxPower(BLE_TX_POWER_DBM);
  Bluefruit.setName(BLE_DEVICE_NAME);
  Bluefruit.Advertising.restartOnDisconnect(false);
  return true;
}

void BleBeacon::update(const BeaconPacket::Payload& p, TriageLevel level) {
  uint8_t buf[BeaconPacket::LENGTH];
  BeaconPacket::encode(p, buf);

  const uint16_t advUnits = intervalUnitsFor(level);

  // Legacy ADV_NONCONN_IND budget: Flags (3B) + Manufacturer Data (14B) + Name (5B) = 22 / 31 bytes.
  if (_started) {
    Bluefruit.Advertising.stop();
  }
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buf, sizeof(buf));
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED);
  Bluefruit.Advertising.setInterval(advUnits, advUnits);
  Bluefruit.Advertising.start(0);

  _started = true;
  _currentIntervalUnits = advUnits;
}

void BleBeacon::stop() {
  if (!_started) return;
  Bluefruit.Advertising.stop();
  _started = false;
}

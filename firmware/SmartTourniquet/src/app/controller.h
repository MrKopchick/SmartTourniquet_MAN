/**
 * @file controller.h
 * @brief Top-level system finite state machine.
 *
 * State transitions:
 *   [STANDBY / System OFF] --Reed closed >= 2s--> [ARMED] --Reed open >= 10s--> [RELEASED]
 *          ^                                         ^                             |
 *          |                                         +--- Reed closed again -------+
 *          +--------------- Long button press (medic reset) -----------------------+
 */
#pragma once
#include <Arduino.h>
#include "../../config.h"
#include "../core/triage.h"
#include "../core/beacon_packet.h"
#include "../drivers/reed_switch.h"
#include "../drivers/button.h"
#include "../drivers/battery.h"
#include "../drivers/oled_display.h"
#include "../services/occlusion_timer.h"
#include "../services/ble_beacon.h"
#include "../services/nfc_tag.h"
#include "../services/power_manager.h"
#include "../services/persistence.h"

class Controller {
 public:
  void begin();
  void loop();

 private:
  enum class Mode : uint8_t { ARMED, RELEASED };

  void arm(uint32_t initialSec, bool restored);
  void release();
  [[noreturn]] void goToStandby();

  BeaconPacket::Payload buildPayload() const;
  uint8_t     buildFlags() const;
  TriageLevel currentLevel() const;

  void publish();       // Broadcast via BLE and NFC
  void persist();       // Snapshot current state into flash
  void showStatus();    // Activate OLED for DISPLAY_ON_MS

  // Hardware drivers and services
  ReedSwitch     _reed;
  Button         _button;
  Battery        _battery;
  OledDisplay    _display;
  OcclusionTimer _timer;
  BleBeacon      _ble;
  NfcTag         _nfc;
  Persistence    _persist;

  // Runtime state
  Mode        _mode = Mode::ARMED;
  uint16_t    _deviceId = 0;
  bool        _timeRestored = false;
  TriageLevel _lastLevel = TriageLevel::SAFE;

  // Task scheduler timestamps
  uint32_t _lastPublishMs = 0;
  uint32_t _lastPersistMs = 0;
  uint32_t _lastBatteryMs = 0;
  uint32_t _displayOffAt = 0;
};

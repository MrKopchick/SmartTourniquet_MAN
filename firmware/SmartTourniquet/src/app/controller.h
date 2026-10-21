// State transitions:
//   [STANDBY / System OFF] --Reed closed >= 2s--> [ARMED] --Reed open >= 10s--> [RELEASED]
//          ^                                         ^                             |
//          |                                         +--- Reed closed again -------+
//          +------- Long button press + confirm press (medic reset) ---------------+
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
  bool        isResetConfirmActive(uint32_t now) const;

  void publish();
  void persist();
  void showStatus();
  void refreshDisplay();

  ReedSwitch     _reed;
  Button         _button;
  Battery        _battery;
  OledDisplay    _display;
  OcclusionTimer _timer;
  BleBeacon      _ble;
  NfcTag         _nfc;
  Persistence    _persist;

  Mode        _mode         = Mode::ARMED;
  uint16_t    _deviceId     = 0;
  bool        _timeRestored = false;
  TriageLevel _lastLevel    = TriageLevel::SAFE;

  uint32_t _lastPublishMs        = 0;
  uint32_t _lastPersistMs        = 0;
  uint32_t _lastRamCacheMs       = 0;
  uint32_t _lastBatteryMs        = 0;
  uint32_t _lastDisplayRefreshMs = 0;
  uint32_t _displayOffAt         = 0;
  uint32_t _resetConfirmUntilMs  = 0;
  bool     _resetConfirmPending  = false;
};

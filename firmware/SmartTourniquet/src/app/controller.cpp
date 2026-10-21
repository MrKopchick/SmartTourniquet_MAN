#include "controller.h"

using namespace BeaconPacket;

void Controller::begin() {
  DBG_BEGIN();
  PowerManager::begin();
  _deviceId = PowerManager::deviceId();
  DBG("[APP] SmartTourniquet v%d.%d ID=%04X reset=0x%08lX\n",
      FW_VERSION_MAJOR, FW_VERSION_MINOR, _deviceId, PowerManager::resetReason());

  _reed.begin(PIN_REED_SWITCH, REED_ARM_CONFIRM_MS, REED_RELEASE_CONFIRM_MS);
  _button.begin(PIN_BUTTON, BUTTON_DEBOUNCE_MS, BUTTON_LONG_PRESS_MS,
                &PowerManager::notifyWakeFromISR);
  _battery.begin();
  _display.begin();
  _persist.begin();

  // Recover active state after brownout / WDT / soft reset; otherwise verify clean wake from System OFF.
  Snapshot snap{};
  if (!PowerManager::wokeFromSystemOff() && _persist.load(snap) && (snap.flags & FLAG_ARMED)) {
    DBG("[APP] Restored %lu s from persistent storage\n",
        static_cast<unsigned long>(snap.elapsedSec));
    arm(snap.elapsedSec, true);
    if (snap.flags & FLAG_RELEASED) {
      _timer.freeze();
      _mode = Mode::RELEASED;
    }
  } else if (_reed.waitStableClosed(REED_ARM_CONFIRM_MS)) {
    arm(0, false);
  } else {
    goToStandby();
  }

  PowerManager::feedWatchdog();
  _ble.begin();
  PowerManager::enableDcDc();
  _nfc.begin();

  publish();
  showStatus();
}

void Controller::loop() {
  PowerManager::feedWatchdog();
  const uint32_t now = millis();

  if (_resetConfirmPending && !isResetConfirmActive(now)) {
    _resetConfirmPending = false;
    if (_display.isOn()) {
      refreshDisplay();
    }
  }

  switch (_reed.update()) {
    case ReedSwitch::Event::OPENED:
      if (_mode == Mode::ARMED) {
        release();
      }
      break;
    case ReedSwitch::Event::CLOSED:
      if (_mode == Mode::RELEASED) {
        _resetConfirmPending = false;
        _timer.resume();
        _mode = Mode::ARMED;
        publish();
        persist();
        showStatus();
      }
      break;
    default:
      break;
  }

  switch (_button.update()) {
    case Button::Event::SHORT_PRESS:
      if (_mode == Mode::RELEASED && isResetConfirmActive(now)) {
        goToStandby();
      }
      showStatus();
      break;
    case Button::Event::LONG_PRESS:
      // Two-step medic reset is permitted only in RELEASED state to prevent accidental erasure.
      if (_mode == Mode::RELEASED) {
        if (isResetConfirmActive(now)) {
          goToStandby();
        } else {
          _resetConfirmPending = true;
          _resetConfirmUntilMs = now + BUTTON_RESET_CONFIRM_MS;
          showStatus();
        }
      } else {
        showStatus();
      }
      break;
    default:
      break;
  }

  if (_display.isOn()) {
    if (static_cast<int32_t>(now - _displayOffAt) >= 0) {
      _display.off();
    } else if (now - _lastDisplayRefreshMs >= DISPLAY_REFRESH_MS) {
      refreshDisplay();
    }
  }

  if (now - _lastRamCacheMs >= 1000UL) {
    _lastRamCacheMs = now;
    _persist.cacheRam(_timer.elapsedSec(), buildFlags());
  }

  const TriageLevel level = currentLevel();
  if (level != _lastLevel || (now - _lastPublishMs >= BLE_PAYLOAD_UPDATE_MS)) {
    _lastLevel = level;
    publish();
  }

  if (now - _lastBatteryMs >= BATTERY_MEASURE_MS) {
    _lastBatteryMs = now;
    _battery.measure();
  }

  if (now - _lastPersistMs >= PERSIST_INTERVAL_MS) {
    persist();
  }

  PowerManager::idle(LOOP_IDLE_MS);
}

void Controller::arm(uint32_t initialSec, bool restored) {
  _timer.start(initialSec);
  _mode                = Mode::ARMED;
  _timeRestored        = restored;
  _lastLevel           = currentLevel();
  _resetConfirmPending = false;
  DBG("[APP] ARMED (t0=%lu s, restored=%d)\n",
      static_cast<unsigned long>(initialSec), restored ? 1 : 0);
  persist();
}

void Controller::release() {
  _timer.freeze();
  _mode = Mode::RELEASED;
  DBG("[APP] RELEASED at %lu s\n", static_cast<unsigned long>(_timer.elapsedSec()));
  publish();
  persist();
}

void Controller::goToStandby() {
  DBG("[APP] Entering STANDBY mode\n");
  _display.off();
  _nfc.stop();
  _ble.stop();
  _persist.clear();
  PowerManager::enterSystemOff(PIN_REED_SWITCH);
}

bool Controller::isResetConfirmActive(uint32_t now) const {
  return _resetConfirmPending && (static_cast<int32_t>(_resetConfirmUntilMs - now) > 0);
}

TriageLevel Controller::currentLevel() const {
  return Triage::fromElapsed(_timer.elapsedSec(), TRIAGE_ATTENTION_SEC, TRIAGE_CRITICAL_SEC);
}

uint8_t Controller::buildFlags() const {
  uint8_t f = FLAG_ARMED;
  switch (currentLevel()) {
    case TriageLevel::ATTENTION:
      f |= FLAG_ATTENTION;
      break;
    case TriageLevel::CRITICAL:
      f |= FLAG_CRITICAL;
      break;
    default:
      break;
  }
  if (_mode == Mode::RELEASED)         f |= FLAG_RELEASED;
  if (_timeRestored)                   f |= FLAG_TIME_RESTORED;
  if (_battery.isLow(BATTERY_LOW_PCT)) f |= FLAG_LOW_BATTERY;
  return f;
}

Payload Controller::buildPayload() const {
  Payload p{};
  p.companyId  = BLE_COMPANY_ID;
  p.deviceId   = _deviceId;
  p.elapsedSec = _timer.elapsedSec();
  p.batteryPct = _battery.percent();
  p.flags      = buildFlags();
  return p;
}

void Controller::publish() {
  _lastPublishMs = millis();
  const Payload p = buildPayload();
  _ble.update(p, currentLevel());
  _nfc.update(p);
}

void Controller::persist() {
  _lastPersistMs = millis();
  _persist.save(_timer.elapsedSec(), buildFlags());
}

void Controller::refreshDisplay() {
  const uint32_t now = millis();
  _lastDisplayRefreshMs = now;

  StatusView v{};
  v.deviceId            = _deviceId;
  v.elapsedSec          = _timer.elapsedSec();
  v.batteryPct          = _battery.percent();
  v.level               = currentLevel();
  v.released            = (_mode == Mode::RELEASED);
  v.timeRestored        = _timeRestored;
  v.lowBattery          = _battery.isLow(BATTERY_LOW_PCT);
  v.resetConfirmPending = isResetConfirmActive(now);
  _display.show(v);
}

void Controller::showStatus() {
  refreshDisplay();
  _displayOffAt = millis() + DISPLAY_ON_MS;
}

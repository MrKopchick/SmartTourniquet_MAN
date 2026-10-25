#include "controller.h"
#include <ctype.h>
#include <string.h>

using namespace BeaconPacket;

void Controller::begin() {
  DBG_BEGIN();
#if DEV_MODE
  {
    const uint32_t t0 = millis();
    while (!Serial && (millis() - t0 < 2000UL)) {
      delay(10);
    }
  }
#endif
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

  PowerManager::feedWatchdog();
  _ble.begin();
  PowerManager::enableDcDc();
  _nfc.begin();

  // Recover active state after brownout / WDT / soft reset; otherwise verify clean wake from System OFF.
#if !DEV_MODE
  Snapshot snap{};
  if (!PowerManager::wokeFromSystemOff() && _persist.load(snap) && (snap.flags & FLAG_ARMED)) {
    DBG("[APP] Restored %lu s from persistent storage\n",
        static_cast<unsigned long>(snap.elapsedSec));
    arm(snap.elapsedSec, true);
    if (snap.flags & FLAG_RELEASED) {
      _timer.freeze();
      _mode = Mode::RELEASED;
    }
  } else
#endif
  if (_reed.waitStableClosed(REED_ARM_CONFIRM_MS)) {
    arm(0, false);
  } else {
#if DEV_MODE
    _persist.clear();
    _mode = Mode::STANDBY;
    _display.off();
    DBG("[DEV] Ready in STANDBY (display OFF). Serial commands: START, STATUS, STOP\n");
#else
    goToStandby();
#endif
  }

  if (_mode != Mode::STANDBY) {
    publish();
    showStatus();
  }
}

void Controller::loop() {
  PowerManager::feedWatchdog();
  const uint32_t now = millis();

#if DEV_MODE
  handleSerialCommands();
#endif

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
      if (_mode == Mode::STANDBY) {
        arm(0, false);
        publish();
        showStatus();
      } else if (_mode == Mode::RELEASED) {
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
      } else if (_mode != Mode::STANDBY) {
        showStatus();
      }
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
      } else if (_mode != Mode::STANDBY) {
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

  if (_mode != Mode::STANDBY) {
    if (now - _lastRamCacheMs >= 1000UL) {
      _lastRamCacheMs = now;
      _persist.cacheRam(_timer.elapsedSec(), buildFlags());
    }

    const TriageLevel level = currentLevel();
    if (level != _lastLevel || (now - _lastPublishMs >= BLE_PAYLOAD_UPDATE_MS)) {
      _lastLevel = level;
      publish();
    }

    if (now - _lastPersistMs >= PERSIST_INTERVAL_MS) {
      persist();
    }
  }

  if (now - _lastBatteryMs >= BATTERY_MEASURE_MS) {
    _lastBatteryMs = now;
    _battery.measure();
  }

  PowerManager::idle(LOOP_IDLE_MS);
}

#if DEV_MODE
void Controller::handleSerialCommands() {
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\n' || c == '\r') {
      if (_cmdLen > 0) {
        _cmdBuf[_cmdLen] = '\0';
        execSerialCommand(_cmdBuf);
        _cmdLen = 0;
      }
    } else if (_cmdLen < sizeof(_cmdBuf) - 1) {
      _cmdBuf[_cmdLen++] = c;
      _lastCmdCharMs = millis();
    }
  }

  if (_cmdLen > 0 && (millis() - _lastCmdCharMs >= 25UL)) {
    _cmdBuf[_cmdLen] = '\0';
    execSerialCommand(_cmdBuf);
    _cmdLen = 0;
  }
}

void Controller::execSerialCommand(char* cmd) {
  while (*cmd && isspace(static_cast<unsigned char>(*cmd))) {
    cmd++;
  }
  size_t len = strlen(cmd);
  while (len > 0 && isspace(static_cast<unsigned char>(cmd[len - 1]))) {
    cmd[--len] = '\0';
  }
  if (len == 0) return;

  for (size_t i = 0; i < len; i++) {
    cmd[i] = static_cast<char>(toupper(static_cast<unsigned char>(cmd[i])));
  }

  if (strcmp(cmd, "START") == 0) {
    if (_mode == Mode::STANDBY) {
      arm(0, false);
      publish();
      showStatus();
      printSerialStatus("START -> ARMED");
    } else if (_mode == Mode::RELEASED) {
      _resetConfirmPending = false;
      _timer.resume();
      _mode = Mode::ARMED;
      publish();
      persist();
      showStatus();
      printSerialStatus("START -> RESUMED");
    } else {
      showStatus();
      printSerialStatus("START (already ARMED)");
    }
  } else if (strcmp(cmd, "STATUS") == 0) {
    if (_mode != Mode::STANDBY) {
      showStatus();
    }
    printSerialStatus("STATUS");
  } else if (strcmp(cmd, "STOP") == 0) {
    if (_mode == Mode::ARMED) {
      release();
      showStatus();
      printSerialStatus("STOP -> RELEASED (send STOP again to reset)");
    } else if (_mode == Mode::RELEASED) {
      goToStandby();
      printSerialStatus("STOP -> STANDBY");
    } else {
      printSerialStatus("STOP (already STANDBY)");
    }
  } else {
    DBG("[DEV] Unknown command '%s'. Use: START, STATUS, STOP\n", cmd);
  }
}

void Controller::printSerialStatus(const char* tag) const {
  const uint32_t sec = _timer.elapsedSec();
  const uint32_t h   = sec / 3600UL;
  const uint32_t m   = (sec / 60UL) % 60UL;
  const uint32_t s   = sec % 60UL;
  const char* modeStr = (_mode == Mode::ARMED)    ? "ARMED"
                      : (_mode == Mode::RELEASED) ? "RELEASED"
                                                  : "STANDBY";
  DBG("[DEV] %s | MODE=%s | TIME=%02lu:%02lu:%02lu | TRIAGE=%s | BAT=%u%% (%umV)\n",
      tag,
      modeStr,
      static_cast<unsigned long>(h),
      static_cast<unsigned long>(m),
      static_cast<unsigned long>(s),
      Triage::label(currentLevel()),
      _battery.percent(),
      _battery.millivolts());
}
#endif

void Controller::arm(uint32_t initialSec, bool restored) {
  if (!_nfc.isReady()) {
    _nfc.begin();
  }
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
  _timer.reset();
  _resetConfirmPending = false;
#if DEV_MODE
  _mode = Mode::STANDBY;
#else
  PowerManager::enterSystemOff(PIN_REED_SWITCH);
#endif
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
  v.standby             = (_mode == Mode::STANDBY);
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

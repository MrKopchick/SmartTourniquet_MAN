#include "controller.h"

using namespace BeaconPacket;

// ============================================================================
//  Initialization
// ============================================================================
void Controller::begin() {
  DBG_BEGIN();
  PowerManager::begin();  // Must execute first to capture RESETREAS before SoftDevice starts
  _deviceId = PowerManager::deviceId();
  DBG("[APP] SmartTourniquet v%d.%d ID=%04X reset=0x%08lX\n",
      FW_VERSION_MAJOR, FW_VERSION_MINOR, _deviceId, PowerManager::resetReason());

  _reed.begin(PIN_REED_SWITCH, REED_ARM_CONFIRM_MS, REED_RELEASE_CONFIRM_MS);
  _button.begin(PIN_BUTTON, BUTTON_DEBOUNCE_MS, BUTTON_LONG_PRESS_MS);
  _battery.begin();
  _display.begin();
  _persist.begin();

  // 1. Check for brownout / watchdog recovery from Flash.
  Snapshot snap;
  if (!PowerManager::wokeFromSystemOff() && _persist.load(snap) && (snap.flags & FLAG_ARMED)) {
    DBG("[APP] Restored %lu s from Flash\n", snap.elapsedSec);
    arm(snap.elapsedSec, true);
    if (snap.flags & FLAG_RELEASED) release();
  }
  // 2. Validate clean wake-up from System OFF via reed switch.
  else if (_reed.waitStableClosed(REED_ARM_CONFIRM_MS)) {
    arm(0, false);
  }
  // 3. Spurious wake-up filter: return directly to deep sleep.
  else {
    goToStandby();
  }

  _ble.begin();
  _nfc.begin();
  publish();
  showStatus();  // Brief visual acknowledgment for operator
}

// ============================================================================
//  Main Loop
// ============================================================================
void Controller::loop() {
  const uint32_t now = millis();

  // --- Reed Switch Event Handling ---
  switch (_reed.update()) {
    case ReedSwitch::Event::OPENED:
      if (_mode == Mode::ARMED) release();
      break;
    case ReedSwitch::Event::CLOSED:
      if (_mode == Mode::RELEASED) {
        _timer.resume();
        _mode = Mode::ARMED;
        publish();
        persist();
      }
      break;
    default:
      break;
  }

  // --- Push Button Event Handling ---
  switch (_button.update()) {
    case Button::Event::SHORT_PRESS:
      showStatus();
      break;
    case Button::Event::LONG_PRESS:
      // Maintenance reset allowed only after deliberate tourniquet release
      if (_mode == Mode::RELEASED) goToStandby();
      break;
    default:
      break;
  }

  // --- OLED Auto-Timeout ---
  if (_display.isOn() && (int32_t)(now - _displayOffAt) >= 0) {
    _display.off();
  }

  // --- Periodic Telemetry & Maintenance Tasks ---
  if (now - _lastBatteryMs >= BATTERY_MEASURE_MS) {
    _lastBatteryMs = now;
    _battery.measure();
  }

  const TriageLevel level = currentLevel();
  if (level != _lastLevel || now - _lastPublishMs >= BLE_PAYLOAD_UPDATE_MS) {
    _lastLevel = level;  // Publish immediately on state transition
    publish();
  }

  if (now - _lastPersistMs >= PERSIST_INTERVAL_MS) {
    persist();
  }

  PowerManager::idle(LOOP_IDLE_MS);
}

// ============================================================================
//  State Transitions
// ============================================================================
void Controller::arm(uint32_t initialSec, bool restored) {
  _timer.start(initialSec);
  _mode = Mode::ARMED;
  _timeRestored = restored;
  DBG("[APP] ARMED (t0=%lu s, restored=%d)\n", initialSec, restored);
  persist();
}

void Controller::release() {
  _timer.freeze();
  _mode = Mode::RELEASED;
  DBG("[APP] RELEASED at %lu s\n", _timer.elapsedSec());
  publish();
  persist();
}

void Controller::goToStandby() {
  DBG("[APP] Entering STANDBY mode\n");
  _display.off();
  _ble.stop();
  _persist.clear();
  PowerManager::enterSystemOff(PIN_REED_SWITCH);
}

// ============================================================================
//  Telemetry & Display
// ============================================================================
TriageLevel Controller::currentLevel() const {
  return Triage::fromElapsed(_timer.elapsedSec(), TRIAGE_ATTENTION_SEC, TRIAGE_CRITICAL_SEC);
}

uint8_t Controller::buildFlags() const {
  uint8_t f = FLAG_ARMED;
  switch (currentLevel()) {
    case TriageLevel::ATTENTION: f |= FLAG_ATTENTION; break;
    case TriageLevel::CRITICAL:  f |= FLAG_CRITICAL;  break;
    default: break;
  }
  if (_mode == Mode::RELEASED)                 f |= FLAG_RELEASED;
  if (_timeRestored)                           f |= FLAG_TIME_RESTORED;
  if (_battery.isLow(BATTERY_LOW_PCT))         f |= FLAG_LOW_BATTERY;
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
  _ble.update(p);
  _nfc.update(p);
}

void Controller::persist() {
  _lastPersistMs = millis();
  _persist.save(_timer.elapsedSec(), buildFlags());
}

void Controller::showStatus() {
  StatusView v{};
  v.deviceId   = _deviceId;
  v.elapsedSec = _timer.elapsedSec();
  v.batteryPct = _battery.percent();
  v.level      = currentLevel();
  v.released   = (_mode == Mode::RELEASED);
  _display.show(v);
  _displayOffAt = millis() + DISPLAY_ON_MS;
}

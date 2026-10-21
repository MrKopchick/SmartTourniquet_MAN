#include "oled_display.h"
#include <Wire.h>

void OledDisplay::begin() {
  pinMode(PIN_EXT_POWER, OUTPUT);
  powerDown();
}

bool OledDisplay::powerUp() {
  digitalWrite(PIN_EXT_POWER, HIGH);
  delay(10);

  Wire.setPins(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.begin();
  Wire.setClock(400000UL);

  _ready = _oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
  if (!_ready) {
    DBG("[OLED] Init failed at 0x%02X\n", OLED_I2C_ADDR);
    Wire.end();
    pinMode(PIN_I2C_SDA, INPUT);
    pinMode(PIN_I2C_SCL, INPUT);
    digitalWrite(PIN_EXT_POWER, LOW);
    _on = false;
    return false;
  }

  _oled.dim(true);
  _on = true;
  return true;
}

void OledDisplay::powerDown() {
  if (_on && _ready) {
    _oled.clearDisplay();
    _oled.display();
    _oled.ssd1306_command(SSD1306_DISPLAYOFF);
    Wire.end();
  }
  // High-Z I2C lines prevent parasitic back-powering through SSD1306 clamp diodes when VCC is cut.
  pinMode(PIN_I2C_SDA, INPUT);
  pinMode(PIN_I2C_SCL, INPUT);
  digitalWrite(PIN_EXT_POWER, LOW);
  _on = false;
  _ready = false;
}

void OledDisplay::drawBatteryIcon(int16_t x, int16_t y, uint8_t pct) {
  _oled.drawRect(x, y, 14, 7, SSD1306_WHITE);
  _oled.fillRect(x + 14, y + 2, 2, 3, SSD1306_WHITE);
  const int16_t fillW = static_cast<int16_t>((static_cast<uint16_t>(pct) * 10U + 50U) / 100U);
  if (fillW > 0) {
    _oled.fillRect(x + 2, y + 2, fillW > 10 ? 10 : fillW, 3, SSD1306_WHITE);
  }
}

static void formatHms(uint32_t sec, char* buf, size_t len) {
  const uint32_t h = sec / 3600UL;
  const uint32_t m = (sec / 60UL) % 60UL;
  const uint32_t s = sec % 60UL;
  snprintf(buf, len, "%02lu:%02lu:%02lu",
           static_cast<unsigned long>(h),
           static_cast<unsigned long>(m),
           static_cast<unsigned long>(s));
}

void OledDisplay::show(const StatusView& v) {
  if (!_on && !powerUp()) return;

  char line[28];
  _oled.clearDisplay();
  _oled.setTextColor(SSD1306_WHITE);

  _oled.setTextSize(1);
  _oled.setCursor(0, 0);
  snprintf(line, sizeof(line), "ID:%04X %s", v.deviceId, v.timeRestored ? "[R]" : "   ");
  _oled.print(line);

  snprintf(line, sizeof(line), "%s%3u%%", v.lowBattery ? "!" : " ", v.batteryPct);
  _oled.setCursor(78, 0);
  _oled.print(line);
  drawBatteryIcon(110, 0, v.batteryPct);

  _oled.drawFastHLine(0, 10, OLED_WIDTH, SSD1306_WHITE);

  _oled.setTextSize(2);
  _oled.setCursor(16, 15);
  formatHms(v.elapsedSec, line, sizeof(line));
  _oled.print(line);

  _oled.drawRect(0, 35, OLED_WIDTH, 6, SSD1306_WHITE);
  const uint16_t permille = Triage::progressPermille(v.elapsedSec, TRIAGE_CRITICAL_SEC);
  const int16_t barWidth = static_cast<int16_t>((static_cast<uint32_t>(OLED_WIDTH - 4) * permille) / 1000UL);
  if (barWidth > 0) {
    _oled.fillRect(2, 37, barWidth, 2, SSD1306_WHITE);
  }

  _oled.setTextSize(1);
  if (v.resetConfirmPending) {
    _oled.fillRect(0, 45, OLED_WIDTH, 19, SSD1306_WHITE);
    _oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    _oled.setCursor(4, 50);
    _oled.print("PRESS AGAIN: STANDBY");
  } else if (v.released) {
    _oled.drawRect(0, 45, OLED_WIDTH, 19, SSD1306_WHITE);
    _oled.setCursor(16, 50);
    _oled.print("STATUS: RELEASED");
  } else if (v.level == TriageLevel::CRITICAL) {
    _oled.fillRect(0, 45, OLED_WIDTH, 19, SSD1306_WHITE);
    _oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    _oled.setCursor(10, 50);
    _oled.print("! CRITICAL >= 2H !");
  } else if (v.level == TriageLevel::ATTENTION) {
    _oled.drawRect(0, 45, OLED_WIDTH, 19, SSD1306_WHITE);
    _oled.setCursor(10, 50);
    _oled.print("ATTENTION (90-120m)");
  } else {
    _oled.setCursor(22, 50);
    _oled.print("STATUS: SAFE");
  }

  _oled.display();
}

void OledDisplay::off() {
  powerDown();
}

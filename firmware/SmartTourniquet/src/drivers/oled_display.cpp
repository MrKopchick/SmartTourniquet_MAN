#include "oled_display.h"
#include <Wire.h>

void OledDisplay::begin() {
  pinMode(PIN_EXT_POWER, OUTPUT);
  powerDown();  // Ensure display is completely unpowered on boot
}

void OledDisplay::powerUp() {
  digitalWrite(PIN_EXT_POWER, HIGH);
  delay(10);  // Allow power rail and internal charge pump to stabilize
  Wire.setPins(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.begin();
  _oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
  _oled.dim(true);  // Dim mode: light discipline and lower current consumption
  _on = true;
}

void OledDisplay::powerDown() {
  if (_on) {
    _oled.ssd1306_command(SSD1306_DISPLAYOFF);
    Wire.end();
  }
  // Set SDA/SCL pins to high impedance to avoid parasitic back-powering via I2C clamp diodes.
  digitalWrite(PIN_EXT_POWER, LOW);
  _on = false;
}

static void formatHms(uint32_t sec, char* buf, size_t len) {
  const uint32_t h = sec / 3600;
  const uint32_t m = (sec / 60) % 60;
  const uint32_t s = sec % 60;
  snprintf(buf, len, "%02lu:%02lu:%02lu", (unsigned long)h, (unsigned long)m, (unsigned long)s);
}

void OledDisplay::show(const StatusView& v) {
  if (!_on) powerUp();

  char line[24];
  _oled.clearDisplay();
  _oled.setTextColor(SSD1306_WHITE);

  // Line 1: Device ID and Battery
  _oled.setTextSize(1);
  _oled.setCursor(0, 0);
  snprintf(line, sizeof(line), "ID %04X    BAT %3u%%", v.deviceId, v.batteryPct);
  _oled.print(line);

  // Line 2: Large format elapsed time
  _oled.setTextSize(2);
  _oled.setCursor(16, 18);
  formatHms(v.elapsedSec, line, sizeof(line));
  _oled.print(line);

  // Line 3: Triage category (CRITICAL inverted for rapid visual identification)
  _oled.setCursor(0, 44);
  if (v.level == TriageLevel::CRITICAL) {
    _oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  }
  _oled.print(v.released ? "RELEASED" : Triage::label(v.level));

  _oled.display();
  // TODO: Add visual indicator for FLAG_TIME_RESTORED state.
}

void OledDisplay::off() {
  powerDown();
}

# Прошивка SmartTourniquet (nRF52840)

## Середовище

1. **Arduino IDE 2.x** → *File → Preferences → Additional Boards Manager URLs*:
   `https://adafruit.github.io/arduino-board-index/package_adafruit_index.json`
2. *Boards Manager* → встановити **Adafruit nRF52**.
3. Плата: для клону Pro Micro / nice!nano — варіант з identity-розпіновкою
   (TODO: зафіксувати конкретний варіант плати після перевірки на залізі).
4. *Library Manager*: **Adafruit SSD1306**, **Adafruit GFX Library**.
   `Bluefruit`, `Adafruit_LittleFS`, `InternalFileSystem` — входять до ядра.
5. Прошивка: подвійне натискання RESET → з'являється диск UF2 → *Upload*.

## Структура

```
SmartTourniquet/
├── SmartTourniquet.ino   setup()/loop() → Controller
├── config.h              піни, пороги TCCC, BLE, таймінги, DEBUG
└── src/
    ├── app/
    │   └── controller.*        кінцевий автомат: STANDBY → ARMED ⇄ RELEASED
    ├── core/                   ЧИСТА логіка (без Arduino) — тестується на ПК
    │   ├── triage.*            SAFE / ATTENTION / CRITICAL за часом
    │   └── beacon_packet.*     серіалізація BLE-пакета (docs/protocol.md)
    ├── drivers/                ЗАЛІЗО
    │   ├── reed_switch.*       геркон з довгим антибрязкітом
    │   ├── button.*            коротке / довге натискання
    │   ├── battery.*           VDD через SAADC → %
    │   └── oled_display.*      SSD1306 «за викликом» + знеструмлення
    └── services/               СИСТЕМНІ СЕРВІСИ
        ├── occlusion_timer.*   хронометраж від RTC (LFXO 32.768 кГц)
        ├── ble_beacon.*        non-connectable advertising
        ├── nfc_tag.*           NFC Type 2 Tag (TODO)
        ├── power_manager.*     System OFF / idle, причина скидання, ID
        └── persistence.*       резервна копія часу у Flash
```

**Правило залежностей:** `app → services, drivers → core`. Модулі одного рівня одне одного не включають.

## Режим налагодження

`config.h` → `DEBUG_SERIAL 1` — лог у Serial Monitor (115200).
Для вимірювання струму (PPK2 / мультиметр) обов'язково `DEBUG_SERIAL 0` і живлення **не** від USB.

## Відкриті TODO (пріоритет)

1. Вимір VDD через SAADC — `drivers/battery.cpp`
2. Струм pull-up через замкнений геркон (~230 мкА!) — `drivers/reed_switch.cpp`
3. NFC Type 2 Tag — `services/nfc_tag.cpp`
4. Переривання кнопки замість опитування — `drivers/button.cpp`
5. WDT, DC/DC — `services/power_manager.cpp`

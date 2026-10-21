# Прошивка SmartTourniquet (nRF52840)

## Середовище та компіляція

1. **Arduino IDE 2.x** → *File → Preferences → Additional Boards Manager URLs*:
   `https://adafruit.github.io/arduino-board-index/package_adafruit_index.json`
2. *Boards Manager* → встановити **Adafruit nRF52**.
3. Плата (*Tools → Board → Adafruit nRF52 Boards*):
   - **Nordic nRF52840 DK (`pca10056`)** — має пряму (1:1) розпіновку портів `P0.xx` / `P1.xx`, що відповідає `NRF_PIN(port, pin)` у `config.h`, та використовує зовнішній кварц `USE_LFXO` (32.768 кГц).
4. Бібліотеки (*Library Manager*): **Adafruit SSD1306**, **Adafruit GFX Library**.
   `Bluefruit52Lib`, `Adafruit_LittleFS`, `InternalFileSystem` входять до складу ядра Adafruit nRF52.
5. Завантаження прошивки: подвійне натискання RESET → з'являється диск UF2 → *Upload* (або копіювання `.uf2`).

Компіляція з командного рядка (`arduino-cli`):
```powershell
arduino-cli compile --fqbn adafruit:nrf52:pca10056 firmware/SmartTourniquet
```

## Структура модулів

```
SmartTourniquet/
├── SmartTourniquet.ino         Точка входу: setup()/loop() → Controller
├── config.h                    Централізована конфігурація: піни, пороги TCCC, BLE, таймінги
└── src/
    ├── app/
    │   └── controller.*        Кінцевий автомат: STANDBY → ARMED ⇄ RELEASED
    ├── core/                   Чиста логіка (без залежностей від заліза)
    │   ├── triage.*            Класифікація SAFE / ATTENTION / CRITICAL + прогрес оклюзії
    │   └── beacon_packet.*     Кодування, декодування та XOR-контрольна сума BLE-пакета
    ├── drivers/                Драйвери периферії
    │   ├── reed_switch.*       Геркон із тривалим антибрязкотом та імпульсним опитуванням pull-up
    │   ├── button.*            Кнопка на перериваннях GPIOTE (коротке / довге натискання)
    │   ├── battery.*           Вимір VDD через SAADC (oversampling 4x + EMA-фільтр) → %
    │   └── oled_display.*      SSD1306 «за викликом» + керування живленням та ізоляція I2C
    └── services/               Системні сервіси
        ├── occlusion_timer.*   64-бітний хронометраж від апаратного RTC (LFXO 32.768 кГц)
        ├── ble_beacon.*        Non-connectable advertising з адаптивним інтервалом за рівнем тріажу
        ├── nfc_tag.*           Емуляція NFC Forum Type 2 Tag (NDEF Text + бінарний запис stq:v1)
        ├── power_manager.*     System OFF / подійний System ON sleep, DC/DC, апаратний WDT
        └── persistence.*       Дворівневе збереження стану (.noinit SRAM + LittleFS Flash з CRC-32)
```

## Ключові реалізовані механізми

1. **Імпульсне опитування геркона (`ReedSwitch`):** при замкненому контакті внутрішня підтяжка (~13 кОм) вмикається лише на 5 мкс під час виміру. Це знижує струм витоку через геркон з ~230 мкА до ~11 нА без потреби в зовнішньому мегаомному резисторі.
2. **Адаптивний BLE-маяк (`BleBeacon`):** інтервал трансляції автоматично зменшується з `1500 мс` (`SAFE`) до `1000 мс` (`ATTENTION`) та `500 мс` (`CRITICAL`) для пришвидшення пеленгації критичних поранених.
3. **Повноцінна емуляція NFC Type 2 Tag (`NfcTag`):** апаратний блок `NRF_NFCT` формує двозаписний NDEF-пакет (людинозрозумілий текстовий рядок для будь-якого смартфона без застосунку + бінарний блок `stq:v1`) із захистом від розірваного читання.
4. **Дворівневий захист від втрати часу (`Persistence`):** щосекундний кеш у `.noinit` SRAM (зберігається при WDT / м'якому скиданні без зносу Flash) та періодичний запис у LittleFS Flash із перевіркою CRC-32.
5. **Двокрокове сервісне скидання медиком (`Controller`):** перехід у `STANDBY` можливий лише у стані `RELEASED` після 5-секундного утримання кнопки та повторного підтвердження протягом 3 секунд.
6. **Стендовий режим масштабування часу (`TIME_SCALE_FACTOR` у `config.h`):** значення `1` — реальний час; `60` — демонстраційний режим для захисту МАН (1 хвилина = 1 година).

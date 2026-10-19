# SmartTourniquet — застосунок медика (Flutter)

Слухає BLE-ефір, розбирає пакети кліпс, формує чергу тріажу (найкритичніші — вгорі)
та допомагає знайти пораненого за RSSI («гаряче-холодно»). NFC — резервний канал.

## Запуск

```bash
cd app/smart_tourniquet_app
flutter create . --platforms=android,ios   # згенерувати платформні папки (один раз)
flutter pub get
flutter run
```

Після `flutter create` додати дозволи:
- **Android** (`android/app/src/main/AndroidManifest.xml`): `BLUETOOTH_SCAN` (`neverForLocation`),
  `ACCESS_FINE_LOCATION` (для Android ≤ 11), `NFC`.
- **iOS** (`ios/Runner/Info.plist`): `NSBluetoothAlwaysUsageDescription`,
  `NFCReaderUsageDescription` + capability *Near Field Communication Tag Reading*.

## Структура `lib/`

```
lib/
├── main.dart                     складання залежностей, запуск
├── core/                         не залежить ні від чого
│   ├── constants.dart            пороги TCCC, Company ID, таймаути
│   ├── format.dart               форматування часу
│   └── models/
│       ├── tourniquet_beacon.dart  один розібраний пакет
│       └── casualty.dart           поранений = останній пакет + екстраполяція часу
├── data/                         джерела даних (залізо телефону)
│   ├── ble/beacon_parser.dart    байти → TourniquetBeacon (дзеркало firmware/core/beacon_packet)
│   ├── ble/ble_scanner.dart      flutter_blue_plus → TriageQueue
│   └── nfc/nfc_reader.dart       NFC → TriageQueue (TODO)
├── domain/                       бізнес-логіка
│   ├── triage_queue.dart         реєстр поранених + сортування
│   └── rssi_locator.dart         згладжування RSSI, тренд «тепліше/холодніше»
└── ui/
    ├── screens/triage_screen.dart
    ├── screens/locator_screen.dart
    └── widgets/casualty_card.dart
```

## Тести

```bash
flutter test     # test/beacon_parser_test.dart — перевірка формату пакета
```

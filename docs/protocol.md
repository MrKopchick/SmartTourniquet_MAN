# Протокол обміну (v1)

> Будь-яка зміна формату: (1) цей документ → (2) `firmware/.../core/beacon_packet.*` →
> (3) `app/.../data/ble/beacon_parser.dart` → (4) `tools/ble_monitor.py` → (5) підняти `PROTOCOL_VERSION`.

## 1. BLE Advertising

- Тип: `ADV_NONCONN_IND` (без з'єднання, без scan response)
- Інтервал: 1000 мс (TODO: адаптивний 2000 / 1000 / 500 мс за рівнем тріажу)
- TX: 0 дБм
- Склад пакета (22 з 31 Б): `Flags` (3) + `Manufacturer Specific Data` (14) + `Complete Local Name "STQ"` (5)

### Manufacturer Specific Data (12 Б, little-endian)

| Зсув | Розмір | Поле | Опис |
|------|--------|------|------|
| 0 | 2 | `company_id` | `0xFFFF` (тестовий) |
| 2 | 1 | `version` | `1` |
| 3 | 2 | `device_id` | молодші 16 біт `FICR->DEVICEID[0]` |
| 5 | 4 | `elapsed_sec` | секунд з моменту накладання |
| 9 | 1 | `battery_pct` | 0–100 |
| 10 | 1 | `flags` | див. нижче |
| 11 | 1 | `checksum` | XOR байтів 0..10 |

### Прапорці

| Біт | Назва | Значення |
|-----|-------|----------|
| 0 | `ARMED` | турнікет накладено |
| 1 | `ATTENTION` | ≥ 90 хв |
| 2 | `CRITICAL` | ≥ 120 хв (ліміт TCCC) |
| 3 | `LOW_BATTERY` | ≤ 20 % |
| 4 | `RELEASED` | турнікет знято; `elapsed_sec` зафіксовано |
| 5 | `TIME_RESTORED` | час відновлено з Flash після перезавантаження (можливе заниження ≤ 60 с + простій) |
| 6–7 | — | резерв |

### Еталонний вектор (для тестів)

`ID=0x0001, elapsed=5025 с (01:23:45), bat=87 %, flags=ARMED|ATTENTION`

```
FF FF | 01 | 01 00 | A1 13 00 00 | 57 | 03 | CS
```

## 2. NFC (Type 2 Tag, NDEF) — TODO

| Запис | Тип | Вміст | Для кого |
|-------|-----|-------|----------|
| 1 | Text (`T`, `uk`) | `STQ 0001 \| T+01:23:45 \| ATTENTION \| BAT 87%` | будь-який смартфон без застосунку |
| 2 | External (`stq:v1`) | 12 байт, як у BLE | застосунок SmartTourniquet |

## 3. Безпека — TODO

Поточна версія не автентифікує пакети (ризик підміни маяків). Напрям: усічений
AES-CMAC (4 Б) з ключем підрозділу + лічильник проти повтору (replay).

# Інструменти

| Файл | Призначення |
|------|-------------|
| `ble_monitor.py` | Живий список кліпс у терміналі ноутбука (Windows/Linux/macOS). Для стенду та налагодження без телефону. |

```powershell
pip install -r requirements.txt
python ble_monitor.py --selftest   # перевірка парсера
python ble_monitor.py              # моніторинг ефіру
```

TODO:
- `power_log.py` — запис профілю струму з Nordic PPK2 (енергобюджет, docs/power_budget.md).
- CSV-лог RSSI/відстань для експериментальної частини роботи МАН.

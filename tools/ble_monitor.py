#!/usr/bin/env python3
"""
ble_monitor.py — Command-line BLE telemetry monitor for SmartTourniquet.

Usage:
    pip install -r requirements.txt
    python ble_monitor.py            # Live telemetry scan
    python ble_monitor.py --selftest # Execute parser verification against reference vector

Protocol specification: docs/protocol.md (mirrors firmware/src/core/beacon_packet.cpp).
"""
import argparse
import asyncio
import struct
import time
from dataclasses import dataclass

COMPANY_ID = 0xFFFF
PROTOCOL_VERSION = 1
ATTENTION_SEC = 90 * 60
CRITICAL_SEC = 120 * 60

FLAGS = {
    0x01: "ARMED",
    0x02: "ATTENTION",
    0x04: "CRITICAL",
    0x08: "LOW_BAT",
    0x10: "RELEASED",
    0x20: "RESTORED",
}


@dataclass
class Beacon:
    device_id: int
    elapsed_sec: int
    battery_pct: int
    flags: int
    rssi: int = 0
    seen_at: float = 0.0


def parse(data: bytes, company_id: int = COMPANY_ID) -> Beacon | None:
    """Decodes 10-byte payload excluding Company ID prefix."""
    if len(data) != 10 or data[0] != PROTOCOL_VERSION:
        return None
    x = (company_id & 0xFF) ^ (company_id >> 8)
    for b in data[:-1]:
        x ^= b
    if x != data[-1]:
        return None
    _, dev_id, elapsed, bat, flags = struct.unpack_from("<BHIBB", data)
    return Beacon(dev_id, elapsed, bat, flags)


def fmt_hms(sec: int) -> str:
    return f"{sec // 3600:02d}:{sec // 60 % 60:02d}:{sec % 60:02d}"


def fmt_flags(flags: int) -> str:
    return ",".join(name for bit, name in FLAGS.items() if flags & bit) or "-"


async def monitor() -> None:
    from bleak import BleakScanner

    seen: dict[int, Beacon] = {}

    def on_adv(_device, adv) -> None:
        raw = adv.manufacturer_data.get(COMPANY_ID)
        if raw is None:
            return
        b = parse(bytes(raw))
        if b:
            b.rssi, b.seen_at = adv.rssi, time.time()
            seen[b.device_id] = b

    async with BleakScanner(detection_callback=on_adv):
        while True:
            await asyncio.sleep(1)
            now = time.time()
            print("\033[2J\033[H", end="")
            print(f"{'ID':>6} {'ELAPSED':>10} {'BAT':>5} {'RSSI':>6} {'AGE':>5}  FLAGS")
            # Sort by longest occlusion time descending (triage priority)
            for b in sorted(seen.values(), key=lambda b: -b.elapsed_sec):
                age = now - b.seen_at
                est = b.elapsed_sec + (0 if b.flags & 0x10 else int(age))
                print(f"{b.device_id:>6X} {fmt_hms(est):>10} {b.battery_pct:>4}% "
                      f"{b.rssi:>5}  {age:>4.0f}s  {fmt_flags(b.flags)}")


def selftest() -> None:
    data = bytes([0x01, 0x01, 0x00, 0xA1, 0x13, 0x00, 0x00, 87, 0x03])
    x = 0xFF ^ 0xFF
    for b in data:
        x ^= b
    b = parse(data + bytes([x]))
    assert b and b.device_id == 1 and b.elapsed_sec == 5025 and b.battery_pct == 87, b
    assert parse(data + bytes([x ^ 1])) is None
    print("selftest OK:", b, fmt_hms(b.elapsed_sec), fmt_flags(b.flags))


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--selftest", action="store_true")
    args = ap.parse_args()
    if args.selftest:
        selftest()
    else:
        asyncio.run(monitor())

import 'package:flutter_test/flutter_test.dart';
import 'package:smart_tourniquet_app/core/models/tourniquet_beacon.dart';
import 'package:smart_tourniquet_app/data/ble/beacon_parser.dart';

/// Canonical test vector matching tools/ble_monitor.py (--selftest).
/// Parameters: ID=0x0001, elapsed=5025 s (01:23:45), bat=87%, flags=ARMED|ATTENTION.
List<int> _vector() {
  final data = [0x01, 0x01, 0x00, 0xA1, 0x13, 0x00, 0x00, 87, 0x03];
  var x = 0xFF ^ 0xFF; // Company ID 0xFFFF
  for (final b in data) {
    x ^= b;
  }
  return [...data, x];
}

void main() {
  test('decodes valid telemetry packet', () {
    final b = BeaconParser.parse(_vector())!;
    expect(b.deviceId, 0x0001);
    expect(b.elapsedSec, 5025);
    expect(b.batteryPct, 87);
    expect(b.flags & BeaconFlags.attention, isNonZero);
  });

  test('rejects corrupted checksum', () {
    final v = _vector()..[4] ^= 0x01;
    expect(BeaconParser.parse(v), isNull);
  });

  test('rejects invalid payload length', () {
    expect(BeaconParser.parse([1, 2, 3]), isNull);
  });
}

import 'dart:async';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import '../../core/constants.dart';
import '../../domain/triage_queue.dart';
import 'beacon_parser.dart';

/// Instantaneous RSSI telemetry sample.
class RssiSample {
  final int deviceId;
  final int rssi;
  final DateTime time;
  const RssiSample(this.deviceId, this.rssi, this.time);
}

/// Passive BLE advertisement scanner. Operates entirely connectionless.
class BleScanner {
  final TriageQueue queue;
  final _rssiController = StreamController<RssiSample>.broadcast();
  StreamSubscription<List<ScanResult>>? _sub;

  BleScanner(this.queue);

  /// Stream of raw incoming RSSI samples per advertisement.
  Stream<RssiSample> get rssiStream => _rssiController.stream;

  Future<void> start() async {
    // TODO: Request runtime permissions (BLUETOOTH_SCAN, ACCESS_FINE_LOCATION on Android <= 11).
    _sub = FlutterBluePlus.onScanResults.listen(_onResults);

    await FlutterBluePlus.startScan(
      withMsd: [MsdFilter(AppConstants.companyId)],  // OS-level hardware filter
      continuousUpdates: true,
      androidScanMode: AndroidScanMode.lowLatency,
    );
  }

  void _onResults(List<ScanResult> results) {
    final now = DateTime.now();
    for (final r in results) {
      final raw = r.advertisementData.manufacturerData[AppConstants.companyId];
      if (raw == null) continue;
      final beacon = BeaconParser.parse(raw);
      if (beacon == null) continue;
      queue.upsert(beacon, r.rssi, now);
      _rssiController.add(RssiSample(beacon.deviceId, r.rssi, now));
    }
  }

  Future<void> stop() async {
    await FlutterBluePlus.stopScan();
    await _sub?.cancel();
    _sub = null;
  }

  Future<void> dispose() async {
    await stop();
    await _rssiController.close();
  }
}

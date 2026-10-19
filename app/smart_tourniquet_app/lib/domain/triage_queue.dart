import 'dart:async';

import 'package:flutter/foundation.dart';

import '../core/models/casualty.dart';
import '../core/models/tourniquet_beacon.dart';

/// Central triage registry and casualty prioritization queue.
///
/// Sort order: CRITICAL -> ATTENTION -> SAFE.
/// Secondary sort: descending occlusion time. Released tourniquets sorted to tail.
class TriageQueue extends ChangeNotifier {
  final Map<int, Casualty> _items = {};
  Timer? _ticker;

  /// Starts periodic 1 Hz timer to advance UI timers during RF loss.
  void startTicker() {
    _ticker ??= Timer.periodic(const Duration(seconds: 1), (_) => notifyListeners());
  }

  void upsert(TourniquetBeacon beacon, int rssi, DateTime now) {
    final existing = _items[beacon.deviceId];
    if (existing == null) {
      _items[beacon.deviceId] = Casualty(
        deviceId: beacon.deviceId,
        lastBeacon: beacon,
        lastSeen: now,
        rssi: rssi,
      );
    } else {
      existing.update(beacon, rssi, now);
    }
  }

  Casualty? byId(int deviceId) => _items[deviceId];

  List<Casualty> sorted([DateTime? at]) {
    final now = at ?? DateTime.now();
    final list = _items.values.toList();
    list.sort((a, b) {
      if (a.lastBeacon.isReleased != b.lastBeacon.isReleased) {
        return a.lastBeacon.isReleased ? 1 : -1;
      }
      final byLevel = b.level(now).index.compareTo(a.level(now).index);
      if (byLevel != 0) return byLevel;
      return b.elapsed(now).compareTo(a.elapsed(now));
    });
    return list;
  }

  void remove(int deviceId) {
    _items.remove(deviceId);
    notifyListeners();
  }

  @override
  void dispose() {
    _ticker?.cancel();
    super.dispose();
  }
}

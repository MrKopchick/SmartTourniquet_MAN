import '../constants.dart';
import 'tourniquet_beacon.dart';

/// Casualty record tracking an occluded tourniquet instance.
///
/// Implements local time extrapolation: derives absolute application time [appliedAt]
/// from received telemetry, allowing accurate progression during RF blackout / jamming.
class Casualty {
  final int deviceId;
  TourniquetBeacon lastBeacon;
  DateTime lastSeen;
  int rssi;

  /// Optional tactical callsign or location label assigned by field medic.
  String? label;

  Casualty({
    required this.deviceId,
    required this.lastBeacon,
    required this.lastSeen,
    required this.rssi,
  });

  /// Estimated absolute timestamp when occlusion was applied.
  DateTime get appliedAt => lastSeen.subtract(Duration(seconds: lastBeacon.elapsedSec));

  /// Current occlusion duration. Freezes value if release flag is active.
  Duration elapsed(DateTime now) => lastBeacon.isReleased
      ? Duration(seconds: lastBeacon.elapsedSec)
      : now.difference(appliedAt);

  bool isStale(DateTime now) => now.difference(lastSeen) > AppConstants.staleAfter;

  TriageLevel level(DateTime now) {
    final t = elapsed(now);
    if (t >= AppConstants.criticalThreshold) return TriageLevel.critical;
    if (t >= AppConstants.attentionThreshold) return TriageLevel.attention;
    return TriageLevel.safe;
  }

  void update(TourniquetBeacon beacon, int newRssi, DateTime now) {
    lastBeacon = beacon;
    lastSeen = now;
    rssi = newRssi;
  }
}

/// Triage priority categories. Higher index represents elevated clinical urgency.
enum TriageLevel { safe, attention, critical }

/// Telemetry status flags (docs/protocol.md).
class BeaconFlags {
  BeaconFlags._();
  static const int armed = 1 << 0;
  static const int attention = 1 << 1;
  static const int critical = 1 << 2;
  static const int lowBattery = 1 << 3;
  static const int released = 1 << 4;
  static const int timeRestored = 1 << 5;
}

/// Immutable decoded telemetry beacon packet.
class TourniquetBeacon {
  final int deviceId;
  final int elapsedSec;
  final int batteryPct;
  final int flags;

  const TourniquetBeacon({
    required this.deviceId,
    required this.elapsedSec,
    required this.batteryPct,
    required this.flags,
  });

  bool get isArmed => flags & BeaconFlags.armed != 0;
  bool get isReleased => flags & BeaconFlags.released != 0;
  bool get isLowBattery => flags & BeaconFlags.lowBattery != 0;
  bool get isTimeRestored => flags & BeaconFlags.timeRestored != 0;

  @override
  String toString() =>
      'Beacon(id=$deviceId, t=${elapsedSec}s, bat=$batteryPct%, flags=0x${flags.toRadixString(16)})';
}

/// Formats duration as HH:MM:SS.
String formatHms(Duration d) {
  final h = d.inHours.toString().padLeft(2, '0');
  final m = (d.inMinutes % 60).toString().padLeft(2, '0');
  final s = (d.inSeconds % 60).toString().padLeft(2, '0');
  return '$h:$m:$s';
}

/// Formats time of day as HH:MM.
String formatClock(DateTime t) =>
    '${t.hour.toString().padLeft(2, '0')}:${t.minute.toString().padLeft(2, '0')}';

/// Formats 16-bit device ID as 4-character uppercase hexadecimal.
String formatDeviceId(int id) => id.toRadixString(16).toUpperCase().padLeft(4, '0');

/// Global configuration constants. Kept in sync with firmware/config.h.
class AppConstants {
  AppConstants._();

  // --- Protocol parameters (docs/protocol.md) ---
  static const int companyId = 0xFFFF;
  static const int protocolVersion = 1;

  /// Length of Manufacturer Data excluding Company ID field
  /// (flutter_blue_plus keys map entries by company ID).
  static const int payloadLength = 10;

  // --- TCCC Triage Thresholds ---
  static const Duration attentionThreshold = Duration(minutes: 90);
  static const Duration criticalThreshold = Duration(minutes: 120);

  // --- RF Telemetry & Connection Timeouts ---
  /// Signal loss timeout threshold: marks device stale while continuing local extrapolation.
  static const Duration staleAfter = Duration(seconds: 30);
}

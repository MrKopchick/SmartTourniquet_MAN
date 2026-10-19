/// Directional proximity trend indicator.
enum ProximityTrend { warmer, colder, steady, unknown }

/// Proximity estimator and directional trend analyzer based on filtered RSSI.
/// Utilizes body shadowing effect at 2.4 GHz as a synthetic directional aperture.
class RssiLocator {
  /// Exponential moving average smoothing factor.
  final double alpha;

  /// Hysteresis threshold in dBm to prevent noisy state oscillation.
  final double hysteresisDb;

  double? _ema;
  double? _reference;
  ProximityTrend _trend = ProximityTrend.unknown;

  RssiLocator({this.alpha = 0.25, this.hysteresisDb = 3.0});

  double? get smoothed => _ema;

  /// Current directional trend relative to reference baseline.
  ProximityTrend get trend => _trend;

  void add(int rssi) {
    _ema = _ema == null ? rssi.toDouble() : alpha * rssi + (1 - alpha) * _ema!;
    _reference ??= _ema;

    final delta = _ema! - _reference!;
    if (delta.abs() < hysteresisDb) {
      _trend = ProximityTrend.steady;
    } else {
      _trend = delta > 0 ? ProximityTrend.warmer : ProximityTrend.colder;
      _reference = _ema;
    }
  }

  /// Estimated distance category based on empirical log-distance path loss approximation.
  String roughDistance() {
    final r = _ema;
    if (r == null) return '—';
    if (r > -55) return '< 1 m';
    if (r > -70) return '1–5 m';
    if (r > -85) return '5–15 m';
    return '> 15 m';
  }

  void reset() {
    _ema = null;
    _reference = null;
    _trend = ProximityTrend.unknown;
  }
}

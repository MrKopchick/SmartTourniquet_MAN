import 'dart:async';

import 'package:flutter/material.dart';

import '../../core/format.dart';
import '../../data/ble/ble_scanner.dart';
import '../../domain/rssi_locator.dart';

/// Proximity and directional search display based on RSSI shielding.
class LocatorScreen extends StatefulWidget {
  final int deviceId;
  final BleScanner scanner;

  const LocatorScreen({super.key, required this.deviceId, required this.scanner});

  @override
  State<LocatorScreen> createState() => _LocatorScreenState();
}

class _LocatorScreenState extends State<LocatorScreen> {
  final _locator = RssiLocator();
  StreamSubscription<RssiSample>? _sub;

  @override
  void initState() {
    super.initState();
    _sub = widget.scanner.rssiStream
        .where((s) => s.deviceId == widget.deviceId)
        .listen((s) => setState(() => _locator.add(s.rssi)));
  }

  @override
  void dispose() {
    _sub?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final (text, color) = switch (_locator.trend) {
      ProximityTrend.warmer => ('CLOSER', Colors.redAccent),
      ProximityTrend.colder => ('FURTHER', Colors.lightBlueAccent),
      ProximityTrend.steady => ('NO CHANGE', Colors.white70),
      ProximityTrend.unknown => ('WAITING FOR SIGNAL...', Colors.white38),
    };
    final rssi = _locator.smoothed;

    return Scaffold(
      appBar: AppBar(title: Text('Search · ID ${formatDeviceId(widget.deviceId)}')),
      body: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Text(text, style: TextStyle(fontSize: 40, fontWeight: FontWeight.bold, color: color)),
            const SizedBox(height: 24),
            Text(rssi == null ? '— dBm' : '${rssi.toStringAsFixed(0)} dBm', style: const TextStyle(fontSize: 32)),
            const SizedBox(height: 8),
            Text('Estimated: ${_locator.roughDistance()}'),
            const SizedBox(height: 32),
            const Padding(
              padding: EdgeInsets.symmetric(horizontal: 32),
              child: Text(
                'Rotate 360 degrees: body shadowing attenuates 2.4 GHz signal from behind, '
                'indicating transmitter line-of-sight.',
                textAlign: TextAlign.center,
              ),
            ),
          ],
        ),
      ),
    );
  }
}

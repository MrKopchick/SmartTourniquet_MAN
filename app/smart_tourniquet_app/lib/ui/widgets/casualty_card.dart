import 'package:flutter/material.dart';

import '../../core/format.dart';
import '../../core/models/casualty.dart';
import '../../core/models/tourniquet_beacon.dart';

/// Casualty summary card widget in triage queue.
class CasualtyCard extends StatelessWidget {
  final Casualty casualty;
  final DateTime now;
  final VoidCallback? onTap;

  const CasualtyCard({super.key, required this.casualty, required this.now, this.onTap});

  static Color colorFor(TriageLevel level) => switch (level) {
        TriageLevel.critical => const Color(0xFFD32F2F),
        TriageLevel.attention => const Color(0xFFF9A825),
        TriageLevel.safe => const Color(0xFF388E3C),
      };

  static String labelFor(TriageLevel level) => switch (level) {
        TriageLevel.critical => 'CRITICAL',
        TriageLevel.attention => 'ATTENTION',
        TriageLevel.safe => 'SAFE',
      };

  @override
  Widget build(BuildContext context) {
    final b = casualty.lastBeacon;
    final level = casualty.level(now);
    final stale = casualty.isStale(now);
    final color = b.isReleased ? Colors.grey : colorFor(level);

    return Card(
      shape: RoundedRectangleBorder(
        side: BorderSide(color: color, width: 3),
        borderRadius: BorderRadius.circular(12),
      ),
      child: ListTile(
        onTap: onTap,
        leading: CircleAvatar(backgroundColor: color, child: Text(formatDeviceId(casualty.deviceId).substring(2))),
        title: Text(
          formatHms(casualty.elapsed(now)),
          style: const TextStyle(fontSize: 28, fontWeight: FontWeight.bold, fontFeatures: [FontFeature.tabularFigures()]),
        ),
        subtitle: Text([
          'ID ${formatDeviceId(casualty.deviceId)}',
          'Applied ~${formatClock(casualty.appliedAt)}',
          'Bat ${b.batteryPct}%',
          if (b.isReleased) 'RELEASED',
          if (b.isTimeRestored) 'TIME RESTORED',
          if (stale) 'SIGNAL LOST (EXTRAPOLATING)',
        ].join(' · ')),
        trailing: Text(b.isReleased ? 'RELEASED' : labelFor(level),
            style: TextStyle(color: color, fontWeight: FontWeight.bold)),
      ),
    );
  }
}

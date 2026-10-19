import 'package:flutter/material.dart';

import '../../data/ble/ble_scanner.dart';
import '../../data/nfc/nfc_reader.dart';
import '../../domain/triage_queue.dart';
import '../widgets/casualty_card.dart';
import 'locator_screen.dart';

/// Primary tactical triage dashboard screen.
class TriageScreen extends StatelessWidget {
  final TriageQueue queue;
  final BleScanner scanner;
  final NfcReader nfc;

  const TriageScreen({super.key, required this.queue, required this.scanner, required this.nfc});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('SmartTourniquet · Triage'),
        actions: [
          IconButton(
            tooltip: 'Scan NFC Tag',
            icon: const Icon(Icons.nfc),
            onPressed: nfc.startSession,
          ),
        ],
      ),
      body: ListenableBuilder(
        listenable: queue,
        builder: (context, _) {
          final now = DateTime.now();
          final items = queue.sorted(now);
          if (items.isEmpty) {
            return const Center(child: Text('Scanning for active tourniquet beacons...'));
          }
          return ListView.builder(
            padding: const EdgeInsets.all(8),
            itemCount: items.length,
            itemBuilder: (context, i) => CasualtyCard(
              casualty: items[i],
              now: now,
              onTap: () => Navigator.of(context).push(MaterialPageRoute(
                builder: (_) => LocatorScreen(deviceId: items[i].deviceId, scanner: scanner),
              )),
            ),
          );
        },
      ),
    );
  }
}

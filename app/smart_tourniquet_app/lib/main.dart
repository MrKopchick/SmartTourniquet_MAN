import 'package:flutter/material.dart';

import 'data/ble/ble_scanner.dart';
import 'data/nfc/nfc_reader.dart';
import 'domain/triage_queue.dart';
import 'ui/screens/triage_screen.dart';

/// Application bootstrap. Constructs domain and data layers via explicit dependency injection.
void main() {
  WidgetsFlutterBinding.ensureInitialized();

  final queue = TriageQueue()..startTicker();
  final scanner = BleScanner(queue);
  final nfc = NfcReader(queue);

  scanner.start();

  runApp(SmartTourniquetApp(queue: queue, scanner: scanner, nfc: nfc));
}

class SmartTourniquetApp extends StatelessWidget {
  final TriageQueue queue;
  final BleScanner scanner;
  final NfcReader nfc;

  const SmartTourniquetApp({super.key, required this.queue, required this.scanner, required this.nfc});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'SmartTourniquet',
      debugShowCheckedModeBanner: false,
      // Dark theme enabled by default for tactical light discipline.
      theme: ThemeData(colorSchemeSeed: Colors.red, brightness: Brightness.dark, useMaterial3: true),
      home: TriageScreen(queue: queue, scanner: scanner, nfc: nfc),
    );
  }
}

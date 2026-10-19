import '../../domain/triage_queue.dart';

/// Secondary backup channel reader using physical NFC tap.
/// Operates under heavy RF jamming or zero-emissions conditions.
class NfcReader {
  final TriageQueue queue;
  NfcReader(this.queue);

  Future<bool> isAvailable() async {
    // TODO: Query platform NFC hardware availability via nfc_manager.
    return false;
  }

  Future<void> startSession() async {
    // TODO: Implement NFC Type 2 Tag NDEF session reader.
  }

  Future<void> stopSession() async {
    // TODO: Terminate active NFC session.
  }
}

import 'dart:typed_data';

import '../../core/constants.dart';
import '../../core/models/tourniquet_beacon.dart';

/// Manufacturer Specific Data decoder. Mirrors firmware/src/core/beacon_packet.cpp.
///
/// Input buffer layout excluding Company ID prefix (10 bytes):
///   [0] ver  [1..2] deviceId  [3..6] elapsedSec  [7] battery  [8] flags  [9] checksum
class BeaconParser {
  BeaconParser._();

  /// Decodes raw manufacturer bytes. Returns null on invalid format, length, or checksum failure.
  static TourniquetBeacon? parse(List<int> data, {int companyId = AppConstants.companyId}) {
    if (data.length != AppConstants.payloadLength) return null;
    if (data[0] != AppConstants.protocolVersion) return null;

    // Checksum verification includes 2-byte Company ID prefix
    var x = (companyId & 0xFF) ^ ((companyId >> 8) & 0xFF);
    for (var i = 0; i < data.length - 1; i++) {
      x ^= data[i];
    }
    if (x != data.last) return null;

    final bd = ByteData.sublistView(Uint8List.fromList(data));
    return TourniquetBeacon(
      deviceId: bd.getUint16(1, Endian.little),
      elapsedSec: bd.getUint32(3, Endian.little),
      batteryPct: data[7],
      flags: data[8],
    );
  }
}

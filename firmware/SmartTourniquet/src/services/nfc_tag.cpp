#include "nfc_tag.h"
#include <nrf.h>
#include <nrf_soc.h>
#include "../../config.h"
#include "../core/triage.h"

namespace {

// NFC Forum Type 2 Tag memory layout: 32 pages x 4 bytes = 128 bytes; READ returns 4 pages (16B).
constexpr size_t kTagPages      = 32;
constexpr size_t kBytesPerPage  = 4;
constexpr size_t kTagMemSize    = kTagPages * kBytesPerPage;
constexpr size_t kReadBlockSize = 16;

constexpr uint8_t kCmdRead = 0x30;
constexpr uint8_t kCmdHlta = 0x50;

alignas(4) uint8_t s_tagMem[kTagMemSize];
alignas(4) uint8_t s_stagingMem[kTagMemSize];
alignas(4) uint8_t s_rxBuf[32];
alignas(4) uint8_t s_txBuf[kReadBlockSize];

volatile bool s_fieldPresent  = false;
volatile bool s_updatePending = false;
uint8_t       s_uid[7]        = {0};

void requestHfclk() {
  uint8_t sd_en = 0;
  (void)sd_softdevice_is_enabled(&sd_en);
  if (sd_en) {
    (void)sd_clock_hfclk_request();
  } else {
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;
  }
}

void releaseHfclk() {
  uint8_t sd_en = 0;
  (void)sd_softdevice_is_enabled(&sd_en);
  if (sd_en) {
    (void)sd_clock_hfclk_release();
  } else {
    NRF_CLOCK->TASKS_HFCLKSTOP = 1;
  }
}

void armRx() {
  NRF_NFCT->PACKETPTR = reinterpret_cast<uint32_t>(s_rxBuf);
  NRF_NFCT->MAXLEN    = sizeof(s_rxBuf);
  NRF_NFCT->RXD.FRAMECONFIG =
      (NFCT_RXD_FRAMECONFIG_PARITY_Parity << NFCT_RXD_FRAMECONFIG_PARITY_Pos) |
      (NFCT_RXD_FRAMECONFIG_SOF_SoF << NFCT_RXD_FRAMECONFIG_SOF_Pos) |
      (NFCT_RXD_FRAMECONFIG_CRCMODERX_CRC16RX << NFCT_RXD_FRAMECONFIG_CRCMODERX_Pos);
  NRF_NFCT->TASKS_ENABLERXDATA = 1;
}

void initTagHeader(uint8_t mem[kTagMemSize]) {
  memset(mem, 0, kTagMemSize);

  // Pages 0..2: 7-byte UID, BCC0 (with Cascade Tag 0x88), BCC1, Internal & static lock bytes.
  mem[0]  = s_uid[0];
  mem[1]  = s_uid[1];
  mem[2]  = s_uid[2];
  mem[3]  = static_cast<uint8_t>(0x88U ^ s_uid[0] ^ s_uid[1] ^ s_uid[2]);
  mem[4]  = s_uid[3];
  mem[5]  = s_uid[4];
  mem[6]  = s_uid[5];
  mem[7]  = s_uid[6];
  mem[8]  = static_cast<uint8_t>(s_uid[3] ^ s_uid[4] ^ s_uid[5] ^ s_uid[6]);
  mem[9]  = 0x48;
  mem[10] = 0xFF;
  mem[11] = 0xFF;

  // Page 3: Capability Container (NDEF magic 0xE1, v1.0, 112B data area, read-only 0x0F).
  mem[12] = 0xE1;
  mem[13] = 0x10;
  mem[14] = 0x0E;
  mem[15] = 0x0F;
}

void buildNdefImage(const BeaconPacket::Payload& p, uint8_t mem[kTagMemSize]) {
  initTagHeader(mem);

  const TriageLevel level =
      Triage::fromElapsed(p.elapsedSec, TRIAGE_ATTENTION_SEC, TRIAGE_CRITICAL_SEC);
  const char* stateStr = (p.flags & BeaconPacket::FLAG_RELEASED) ? "RELEASED" : Triage::label(level);

  const uint32_t h = p.elapsedSec / 3600UL;
  const uint32_t m = (p.elapsedSec / 60UL) % 60UL;
  const uint32_t s = p.elapsedSec % 60UL;

  char textBuf[56];
  const int textLen = snprintf(textBuf, sizeof(textBuf),
                               "STQ %04X | T+%02lu:%02lu:%02lu | %s | BAT %u%%",
                               p.deviceId,
                               static_cast<unsigned long>(h),
                               static_cast<unsigned long>(m),
                               static_cast<unsigned long>(s),
                               stateStr,
                               p.batteryPct);
  const uint8_t safeTextLen =
      (textLen > 0 && static_cast<size_t>(textLen) < sizeof(textBuf))
          ? static_cast<uint8_t>(textLen)
          : 0U;

  // Record 1: Well-Known Text ("T", UTF-8, "en"), header 0x91 (MB=1, ME=0, SR=1, TNF=0x01).
  const uint8_t rec1PayloadLen = static_cast<uint8_t>(1U + 2U + safeTextLen);
  const uint8_t rec1TotalLen   = static_cast<uint8_t>(3U + 1U + rec1PayloadLen);

  // Record 2: External Type ("stq:v1", 12B binary payload), header 0x54 (MB=0, ME=1, SR=1, TNF=0x04).
  static const char kExtType[] = "stq:v1";
  constexpr uint8_t kExtTypeLen = 6;
  constexpr uint8_t rec2PayloadLen = BeaconPacket::LENGTH;
  constexpr uint8_t rec2TotalLen   = 3U + kExtTypeLen + rec2PayloadLen;

  const uint8_t ndefMsgLen = static_cast<uint8_t>(rec1TotalLen + rec2TotalLen);

  size_t idx = 16;
  mem[idx++] = 0x03;
  mem[idx++] = ndefMsgLen;

  mem[idx++] = 0x91;
  mem[idx++] = 0x01;
  mem[idx++] = rec1PayloadLen;
  mem[idx++] = 'T';
  mem[idx++] = 0x02;
  mem[idx++] = 'e';
  mem[idx++] = 'n';
  memcpy(&mem[idx], textBuf, safeTextLen);
  idx += safeTextLen;

  mem[idx++] = 0x54;
  mem[idx++] = kExtTypeLen;
  mem[idx++] = rec2PayloadLen;
  memcpy(&mem[idx], kExtType, kExtTypeLen);
  idx += kExtTypeLen;
  BeaconPacket::encode(p, &mem[idx]);
  idx += rec2PayloadLen;

  if (idx < kTagMemSize) {
    mem[idx] = 0xFE;
  }
}

}  // namespace

extern "C" void NFCT_IRQHandler(void) {
  if (NRF_NFCT->EVENTS_FIELDDETECTED) {
    NRF_NFCT->EVENTS_FIELDDETECTED = 0;
    s_fieldPresent = true;
    requestHfclk();
    NRF_NFCT->TASKS_ACTIVATE = 1;
  }

  if (NRF_NFCT->EVENTS_SELECTED) {
    NRF_NFCT->EVENTS_SELECTED   = 0;
    NRF_NFCT->EVENTS_RXFRAMEEND = 0;
    NRF_NFCT->EVENTS_RXERROR    = 0;
    armRx();
  }

  if (NRF_NFCT->EVENTS_RXFRAMEEND) {
    NRF_NFCT->EVENTS_RXFRAMEEND = 0;

    if (NRF_NFCT->FRAMESTATUS.RX == 0) {
      const uint8_t cmd = s_rxBuf[0];
      if (cmd == kCmdRead) {
        const uint8_t startPage = s_rxBuf[1];
        for (size_t i = 0; i < kReadBlockSize; i++) {
          const size_t byteIndex = (static_cast<size_t>(startPage) * kBytesPerPage + i) % kTagMemSize;
          s_txBuf[i] = s_tagMem[byteIndex];
        }
        NRF_NFCT->PACKETPTR = reinterpret_cast<uint32_t>(s_txBuf);
        NRF_NFCT->TXD.FRAMECONFIG =
            (NFCT_TXD_FRAMECONFIG_PARITY_Parity << NFCT_TXD_FRAMECONFIG_PARITY_Pos) |
            (NFCT_TXD_FRAMECONFIG_DISCARDMODE_DiscardStart << NFCT_TXD_FRAMECONFIG_DISCARDMODE_Pos) |
            (NFCT_TXD_FRAMECONFIG_SOF_SoF << NFCT_TXD_FRAMECONFIG_SOF_Pos) |
            (NFCT_TXD_FRAMECONFIG_CRCMODETX_CRC16TX << NFCT_TXD_FRAMECONFIG_CRCMODETX_Pos);
        NRF_NFCT->TXD.AMOUNT    = (kReadBlockSize << NFCT_TXD_AMOUNT_TXDATABYTES_Pos);
        NRF_NFCT->TASKS_STARTTX = 1;
      } else if (cmd == kCmdHlta) {
        NRF_NFCT->TASKS_GOSLEEP = 1;
      } else {
        // 4-bit NACK (0x00) for unsupported/write commands on read-only tag.
        s_txBuf[0]          = 0x00;
        NRF_NFCT->PACKETPTR = reinterpret_cast<uint32_t>(s_txBuf);
        NRF_NFCT->TXD.FRAMECONFIG =
            (NFCT_TXD_FRAMECONFIG_SOF_SoF << NFCT_TXD_FRAMECONFIG_SOF_Pos);
        NRF_NFCT->TXD.AMOUNT    = (4UL << NFCT_TXD_AMOUNT_TXDATABITS_Pos);
        NRF_NFCT->TASKS_STARTTX = 1;
      }
    } else {
      NRF_NFCT->FRAMESTATUS.RX = 0xFFFFFFFFUL;
      armRx();
    }
  }

  if (NRF_NFCT->EVENTS_TXFRAMEEND) {
    NRF_NFCT->EVENTS_TXFRAMEEND = 0;
    armRx();
  }

  if (NRF_NFCT->EVENTS_RXERROR) {
    NRF_NFCT->EVENTS_RXERROR = 0;
    NRF_NFCT->FRAMESTATUS.RX = 0xFFFFFFFFUL;
    armRx();
  }

  if (NRF_NFCT->EVENTS_ERROR) {
    NRF_NFCT->EVENTS_ERROR = 0;
    NRF_NFCT->ERRORSTATUS  = 0xFFFFFFFFUL;
  }

  if (NRF_NFCT->EVENTS_FIELDLOST) {
    NRF_NFCT->EVENTS_FIELDLOST = 0;
    s_fieldPresent = false;
    if (s_updatePending) {
      memcpy(s_tagMem, s_stagingMem, kTagMemSize);
      s_updatePending = false;
    }
    releaseHfclk();
    NRF_NFCT->TASKS_SENSE = 1;
  }
}

bool NfcTag::begin() {
  if ((NRF_UICR->NFCPINS & UICR_NFCPINS_PROTECT_Msk) !=
      (UICR_NFCPINS_PROTECT_NFC << UICR_NFCPINS_PROTECT_Pos)) {
    DBG("[NFC] Warning: UICR.NFCPINS configured as GPIO; NFCT antenna pins disabled\n");
    _ready = false;
    return false;
  }

  NRF_NFCT->TASKS_DISABLE = 1;

  // Derive 7-byte NFCID1 from factory FICR device ID + Nordic manufacturer ID (0x5F).
  const uint32_t id0 = NRF_FICR->DEVICEID[0];
  const uint32_t id1 = NRF_FICR->DEVICEID[1];
  s_uid[0] = 0x5F;
  s_uid[1] = static_cast<uint8_t>((id1 >> 8) & 0xFFU);
  s_uid[2] = static_cast<uint8_t>(id1 & 0xFFU);
  s_uid[3] = static_cast<uint8_t>((id0 >> 24) & 0xFFU);
  s_uid[4] = static_cast<uint8_t>((id0 >> 16) & 0xFFU);
  s_uid[5] = static_cast<uint8_t>((id0 >> 8) & 0xFFU);
  s_uid[6] = static_cast<uint8_t>(id0 & 0xFFU);

  NRF_NFCT->NFCID1_2ND_LAST =
      (static_cast<uint32_t>(s_uid[0]) << 16) |
      (static_cast<uint32_t>(s_uid[1]) << 8) |
      static_cast<uint32_t>(s_uid[2]);
  NRF_NFCT->NFCID1_LAST =
      (static_cast<uint32_t>(s_uid[3]) << 24) |
      (static_cast<uint32_t>(s_uid[4]) << 16) |
      (static_cast<uint32_t>(s_uid[5]) << 8) |
      static_cast<uint32_t>(s_uid[6]);

  NRF_NFCT->SENSRES =
      (NFCT_SENSRES_NFCIDSIZE_NFCID1Double << NFCT_SENSRES_NFCIDSIZE_Pos) |
      (NFCT_SENSRES_BITFRAMESDD_SDD00100 << NFCT_SENSRES_BITFRAMESDD_Pos);
  NRF_NFCT->SELRES           = 0x00UL;
  NRF_NFCT->AUTOCOLRESCONFIG = (NFCT_AUTOCOLRESCONFIG_MODE_Enabled << NFCT_AUTOCOLRESCONFIG_MODE_Pos);

  NRF_NFCT->FRAMEDELAYMIN  = 0x00000480UL;
  NRF_NFCT->FRAMEDELAYMAX  = 0x00001000UL;
  NRF_NFCT->FRAMEDELAYMODE =
      (NFCT_FRAMEDELAYMODE_FRAMEDELAYMODE_WindowGrid << NFCT_FRAMEDELAYMODE_FRAMEDELAYMODE_Pos);

  initTagHeader(s_tagMem);
  initTagHeader(s_stagingMem);

  NRF_NFCT->EVENTS_FIELDDETECTED = 0;
  NRF_NFCT->EVENTS_FIELDLOST     = 0;
  NRF_NFCT->EVENTS_SELECTED      = 0;
  NRF_NFCT->EVENTS_RXFRAMEEND    = 0;
  NRF_NFCT->EVENTS_TXFRAMEEND    = 0;
  NRF_NFCT->EVENTS_RXERROR       = 0;
  NRF_NFCT->EVENTS_ERROR         = 0;
  NRF_NFCT->ERRORSTATUS          = 0xFFFFFFFFUL;

  NRF_NFCT->INTENSET =
      NFCT_INTENSET_FIELDDETECTED_Msk |
      NFCT_INTENSET_FIELDLOST_Msk |
      NFCT_INTENSET_SELECTED_Msk |
      NFCT_INTENSET_RXFRAMEEND_Msk |
      NFCT_INTENSET_TXFRAMEEND_Msk |
      NFCT_INTENSET_RXERROR_Msk |
      NFCT_INTENSET_ERROR_Msk;

  NVIC_ClearPendingIRQ(NFCT_IRQn);
  NVIC_SetPriority(NFCT_IRQn, 3);
  NVIC_EnableIRQ(NFCT_IRQn);

  NRF_NFCT->TASKS_SENSE = 1;
  _ready = true;
  return true;
}

void NfcTag::update(const BeaconPacket::Payload& p) {
  if (!_ready) return;

  buildNdefImage(p, s_stagingMem);

  NVIC_DisableIRQ(NFCT_IRQn);
  if (s_fieldPresent) {
    // Defer copy until reader field is removed to prevent torn multi-page reads.
    s_updatePending = true;
  } else {
    memcpy(s_tagMem, s_stagingMem, kTagMemSize);
    s_updatePending = false;
  }
  NVIC_EnableIRQ(NFCT_IRQn);
}

void NfcTag::stop() {
  if (!_ready) return;
  NVIC_DisableIRQ(NFCT_IRQn);
  NRF_NFCT->INTENCLR      = 0xFFFFFFFFUL;
  NRF_NFCT->TASKS_DISABLE = 1;
  if (s_fieldPresent) {
    s_fieldPresent = false;
    releaseHfclk();
  }
  _ready = false;
}

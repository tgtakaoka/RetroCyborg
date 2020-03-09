/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __mc6850_h__
#define __mc6850_h__

#include <Arduino.h>
#include <stdint.h>

class Mc6850 {
public:
  Mc6850(
    HardwareSerial &serial, uint16_t baseAddr, uint8_t rxInt, uint8_t txInt)
    : _serial(serial),
      _baseAddr(baseAddr),
      _rxInt(rxInt),
      _txInt(txInt),
      _control(CDS_DIV1_gc),
      _status(TDRE_bm),
      _readFlags(0),
      _nextFlags(0),
      _txData(0),
      _rxData(0)
  {}

  bool isSelected(uint16_t addr) const {
    return addr == _baseAddr || --addr == _baseAddr;
  }

  void write(uint8_t data, uint16_t addr);
  uint8_t read(uint16_t addr);
  void loop();

private:
  HardwareSerial &_serial;
  const uint16_t _baseAddr;
  const uint8_t _rxInt;
  const uint8_t _txInt;
  uint8_t _control;
  uint8_t _status;
  uint8_t _readFlags;
  uint8_t _nextFlags;
  uint8_t _txData;
  uint8_t _rxData;

  // Bit Definition of control register
  static constexpr uint8_t CDS_gm       = 0x03; // Counter Divider Select group mask
  static constexpr uint8_t CDS_DIV1_gc  = 0x00; // Counter Divider /1
  static constexpr uint8_t CDS_DIV16_gc = 0x01; // Counter Divider /16
  static constexpr uint8_t CDS_DIV64_gc = 0x02; // Counter Divider /64
  static constexpr uint8_t CDS_RESET_gc = 0x03; // Master Reset
  static constexpr uint8_t TCB_gm       = 0x60; // Transmit Control group mask
  static constexpr uint8_t TCB_DI_gc    = 0x00; // RTS=Low,  Tx Interrupt Disabled
  static constexpr uint8_t TCB_EI_gc    = 0x20; // RTS=LOW,  Tx Interrupt Enabled
  static constexpr uint8_t TCB_RTS_gc   = 0x40; // RTS=High, Tx Interrupt Disabled
  static constexpr uint8_t TCB_BREAK_gc = 0x60; // RTS=Low,  Tx Interrupt Disabled
                                                // Transmit Break Level
  static constexpr uint8_t RIEB_bm = 0x80; // Receive Interrupt Enable bit mask

  static uint8_t cds(uint8_t control) { return control & CDS_gm; }
  static uint8_t tcb(uint8_t control) { return control & TCB_gm; }
  static uint8_t rieb(uint8_t control) { return control & RIEB_bm; }
  bool masterReset() const { return cds(_control) == CDS_RESET_gc; }
  bool txIntEnabled() const { return tcb(_control) == TCB_EI_gc; }
  uint8_t rxIntEnabled() const { return rieb(_control); }
  uint8_t txRegEmpty() const { return _status & TDRE_bm; }
  uint8_t rxRegFull() const { return _status & RDRF_bm; }
  void assertIrq(uint8_t intMask);
  void negateIrq(uint8_t intMask);

  // Bit definition of status register
  // DCD, CTS, FERR, PERR are always zero.
  static constexpr uint8_t RDRF_bm = 0x01; // Receive Data Register Full
  static constexpr uint8_t TDRE_bm = 0x02; // Transmit Data Register Empty
  static constexpr uint8_t DCD_bm  = 0x04; // Data Carieer Detect
  static constexpr uint8_t CTS_bm  = 0x08; // Clear to Send
  static constexpr uint8_t FERR_bm = 0x10; // Framing Error
  static constexpr uint8_t OVRN_bm = 0x20; // Receiver Overrun
  static constexpr uint8_t PEER_bm = 0x40; // Parity Error
  static constexpr uint8_t IRQF_bm = 0x80; // Interrupt Request
};

#endif

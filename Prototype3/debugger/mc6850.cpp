#include "mc6850.h"

#include "pins.h"

//#define DEBUG 1

Mc6850::Mc6850(uint16_t baseAddr)
  : _baseAddr(baseAddr),
    _rxIrq(Pins::getIrqMask(baseAddr)),
    _txIrq(Pins::getIrqMask(baseAddr + 1)),
    _control(0),
    _status(TDRE),
    _readFlags(0),
    _nextFlags(0),
    _receivedData(0),
    _sendingData(0)
{}

void Mc6850::assertIrq(uint8_t irq) {
  _status |= IRQF;
  Pins.assertIrq(irq);
#if DEBUG
  if (irq & _rxIrq) Serial.println(F("@@ Assert RX IRQ"));
  if (irq & _txIrq) Serial.println(F("@@ Assert TX IRQ"));
#endif
}

void Mc6850::negateIrq(uint8_t irq) {
  Pins.negateIrq(irq);
  if (Pins.irqStatus(_txIrq | _rxIrq) == 0)
    _status &= ~IRQF;
#if DEBUG
  if (irq & _rxIrq) Serial.println(F("@@ Negate RX IRQ"));
  if (irq & _txIrq) Serial.println(F("@@ Negate TX IRQ"));
#endif
}

void Mc6850::loop() {
  if (Serial.available() > 0) {
    _receivedData = Serial.read();
    if (_status & RDRF)
      _nextFlags |= OVRN;
    _status |= RDRF;
    if (rxIntEnabled())
      assertIrq(_rxIrq);
  }
  if (Serial.availableForWrite() > 0) {
    if ((_status & TDRE) == 0) {
      Serial.write(_sendingData);
      _status |= TDRE;
      if (txIntEnabled())
        assertIrq(_txIrq);
    }
  }
}

void Mc6850::write(uint8_t data, uint16_t addr) {
  if (addr == _baseAddr) {
    const uint8_t cds = data & (CDS1 | CDS0);
    if (cds == 0) {           // master reset
      negateIrq(_rxIrq | _txIrq);
      _status = TDRE;
      _nextFlags = _readFlags = 0;
    }
    _control = data;
  } else {
#if DEBUG
    Serial.print(F("@@ Write 0x"));
    Serial.print(data, HEX);
    Serial.print(F(" 0x"));
    Serial.println(_status, HEX);
#endif
    _sendingData = data;
    _status &= ~TDRE;
    if (txIntEnabled())
      negateIrq(_txIrq);
  }
}

uint8_t Mc6850::read(uint16_t addr) {
  if (addr == _baseAddr) {
    _readFlags = _status & (RDRF | OVRN);
    return _status;
  } else {
#if DEBUG
    Serial.print(F("@@  Read 0x"));
    Serial.print(_receivedData, HEX);
    Serial.print(F(" 0x")); Serial.print(_status, HEX);
    Serial.print(F(" 0x")); Serial.print(_readFlags, HEX);
    Serial.print(F(" 0x")); Serial.println(_nextFlags, HEX);
#endif
    _status &= ~_readFlags;
    _status |= _nextFlags;
    _readFlags = _nextFlags = 0;
    if (rxIntEnabled()) {
      if (_status & (RDRF | OVRN)) {
        assertIrq(_rxIrq);
      } else {
        negateIrq(_rxIrq);
      }
    }
    return _receivedData;
  }
}

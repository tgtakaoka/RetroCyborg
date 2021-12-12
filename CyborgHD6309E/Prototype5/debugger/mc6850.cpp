/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "mc6850.h"

#include <libcli.h>

#include "pins.h"

//#define DEBUG_IRQ
//#define DEBUG_STATUS
//#define DEBUG_CONTROL
//#define DEBUG_READ
//#define DEBUG_WRITE

extern libcli::Cli &cli;

Mc6850::Mc6850(HardwareSerial &serial)
    : _serial(serial),
      _control(CDS_DIV1_gc),
      _status(TDRE_bm),
      _readFlags(0),
      _nextFlags(0),
      _txData(0),
      _rxData(0),
      _enabled(false) {
    _rxInt = Pins.allocateIrq();
    _txInt = Pins.allocateIrq();
}

void Mc6850::enable(bool enabled, uint16_t baseAddr) {
    _enabled = enabled;
    _baseAddr = baseAddr & ~1;
}

void Mc6850::assertIrq(uint8_t irq) {
    _status |= IRQF_bm;
    Pins.assertIrq(irq);
#ifdef DEBUG_IRQ
    if (irq == _rxInt)
        cli.println(F("@@ Assert RX INT"));
    if (irq == _txInt)
        cli.println(F("@@ Assert TX INT"));
#endif
}

void Mc6850::negateIrq(uint8_t irq) {
    Pins.negateIrq(irq);
    _status &= ~IRQF_bm;
#ifdef DEBUG_IRQ
    if (irq == _rxInt)
        cli.println(F("@@ Negate RX IRQ"));
    if (irq == _txInt)
        cli.println(F("@@ Negate TX IRQ"));
#endif
}

void Mc6850::loop() {
    if (!_enabled)
        return;
    if (_serial.available() > 0) {
        const uint8_t data = _serial.read();
        cli();
        _rxData = data;
        if (rxRegFull())
            _nextFlags |= OVRN_bm;
        _status |= RDRF_bm;
        if (rxIntEnabled())
            assertIrq(_rxInt);
        sei();
#ifdef DEBUG_READ
        cli.print(F("@@ Recv "));
        cli.printHex(_rxData, 2);
        cli.print(' ');
        cli.printlnHex(_status, 2);
#endif
    }
    // TODO: Implement flow control
    if (_serial.availableForWrite() > 0) {
        if (!txRegEmpty()) {
            cli();
            const uint8_t data = _txData;
            _status |= TDRE_bm;
            if (txIntEnabled())
                assertIrq(_txInt);
            sei();
            _serial.write(data);
#ifdef DEBUG_WRITE
            cli.print(F("@@ Send "));
            cli.printHex(_txData, 2);
            cli.print(' ');
            cli.printlnHex(_status, 2);
#endif
        }
    }
}

void Mc6850::write(uint8_t data, uint16_t addr) {
    if (addr == _baseAddr) {
#ifdef DEBUG_CONTROL
        cli.print(F("@@ Control "));
        cli.printlnHex(data, 2);
#endif
        const uint8_t delta = _control ^ data;
        _control = data;
        if (cds(delta) && masterReset()) {
            negateIrq(_txInt | _rxInt);
            _status = TDRE_bm;
            _readFlags = _nextFlags = 0;
        }
        if (tcb(delta)) {
            // TODO: flow control
            if (txIntEnabled() && txRegEmpty()) {
                assertIrq(_txInt);
            } else {
                negateIrq(_txInt);
            }
        }
        if (rieb(delta)) {
            if (rxIntEnabled() && rxRegFull()) {
                assertIrq(_rxInt);
            } else {
                negateIrq(_rxInt);
            }
        }
    } else {
        _txData = data;
        _status &= ~TDRE_bm;
        if (txIntEnabled())
            negateIrq(_txInt);
#ifdef DEBUG_WRITE
        cli.print(F("@@ Write "));
        cli.printHex(data, 2);
        cli.print(' ');
        cli.printHex(_status, 2);
        cli.println();
#endif
    }
}

uint8_t Mc6850::read(uint16_t addr) {
    if (addr == _baseAddr) {
        _readFlags = _status & (DCD_bm | OVRN_bm);
#ifdef DEBUG_STATUS
        if (rxRegFull() || !txRegEmpty()) {
            cli.print(F("@@ Status "));
            cli.printHex(_status, 2);
            cli.println();
        }
#endif
        return _status;
    } else {
#ifdef DEBUG_READ
        const uint8_t prev_status = _status;
#endif
        _status &= ~(RDRF_bm | _readFlags);
        _status |= _nextFlags;
        _readFlags = _nextFlags = 0;
#ifdef DEBUG_READ
        cli.print(F("@@ Read "));
        cli.printHex(_rxData, 2);
        cli.print(' ');
        cli.printHex(prev_status, 2);
        cli.print(F("->"));
        cli.printHex(_status, 2);
        cli.println();
#endif
        if (rxIntEnabled()) {
            if (_status & (RDRF_bm | OVRN_bm)) {
                assertIrq(_rxInt);
            } else {
                negateIrq(_rxInt);
            }
        }
        return _rxData;
    }
}

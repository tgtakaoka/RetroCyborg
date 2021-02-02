/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "mc6850.h"

#include <libcli.h>

#include "pins.h"

//#define DEBUG_IRQ
//#define DEBUG_STATUS
//#define DEBUG_CONTROL
//#define DEBUG_READ
//#define DEBUG_WRITE

extern class libcli::Cli Cli;

void Mc6850::assertIrq(uint8_t intMask) {
    _status |= IRQF_bm;
    Pins.assertIrq(intMask);
#ifdef DEBUG_IRQ
    if (intMask & _rxInt)
        Cli.println(F("@@ Assert RX INT"));
    if (intMask & _txInt)
        Cli.println(F("@@ Assert TX INT"));
#endif
}

void Mc6850::negateIrq(uint8_t intMask) {
    Pins.negateIrq(intMask);
    _status &= ~IRQF_bm;
#ifdef DEBUG_IRQ
    if (intMask & _rxInt)
        Cli.println(F("@@ Negate RX IRQ"));
    if (intMask & _txInt)
        Cli.println(F("@@ Negate TX IRQ"));
#endif
}

void Mc6850::loop() {
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
        Cli.print(F("@@ Recv "));
        Cli.printHex8(_rxData);
        Cli.print(' ');
        Cli.printHex8(_status);
        Cli.println();
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
            Cli.print(F("@@ Send "));
            Cli.printHex8(_txData);
            Cli.print(' ');
            Cli.printHex8(_status);
            Cli.println();
#endif
        }
    }
}

void Mc6850::write(uint8_t data, uint16_t addr) {
    if (addr == _baseAddr) {
#ifdef DEBUG_CONTROL
        Cli.print(F("@@ Control "));
        Cli.printHex8(data);
        Cli.println();
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
        Cli.print(F("@@ Write "));
        Cli.printHex8(data);
        Cli.print(' ');
        Cli.printHex8(_status);
        Cli.println();
#endif
    }
}

uint8_t Mc6850::read(uint16_t addr) {
    if (addr == _baseAddr) {
        _readFlags = _status & (DCD_bm | OVRN_bm);
#ifdef DEBUG_STATUS
        if (rxRegFull() || !txRegEmpty()) {
            Cli.print(F("@@ Status "));
            Cli.printHex8(_status);
            Cli.println();
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
        Cli.print(F("@@ Read "));
        Cli.printHex8(_rxData);
        Cli.print(' ');
        Cli.printHex8(prev_status);
        Cli.print(F("->"));
        Cli.printHex8(_status);
        Cli.println();
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

#include "i8251.h"

#include <libcli.h>

//#define DEBUG_INTR
//#define DEBUG_STATUS
//#define DEBUG_COMMAND
//#define DEBUG_READ
//#define DEBUG_WRITE

extern libcli::Cli cli;

I8251::I8251(Stream &stream) : _stream(stream), _enabled(false) {
    reset();
}

void I8251::reset() {
    _state = STATE_MODE;
    _mode = 0;
    _status = ST_TxRDY_bm | ST_TxEMPTY_bm;
    _command = 0;
    _txData = _rxData = 0;
    negateIntr(_rxIntr);
    negateIntr(_txIntr);
    _rxIntr = _txIntr = INTR_NONE;
}

void I8251::enable(bool enabled, uint16_t baseAddr) {
    _enabled = enabled;
    _baseAddr = baseAddr & ~3;
    reset();
#if defined(DEBUG_STATUS) || defined(DEBUG_COMMAND)
    cli.print("@@ Enable=");
    cli.printDec(enabled);
    cli.print("  baseAddr=");
    cli.printlnHex(_baseAddr);
#endif
}

void I8251::assertIntr(IntrName intr) const {
    Pins.assertIntr(intr);
#ifdef DEBUG_INTR
    if (intr && intr == _rxIntr) {
        cli.print(F("@@ Assert RX INTR "));
        cli.printlnHex(intr, 2);
    }
    if (intr && intr == _txIntr) {
        cli.print(F("@@ Assert TX INTR "));
        cli.printlnHex(intr, 2);
    }
#endif
}

void I8251::negateIntr(IntrName intr) const {
    Pins.negateIntr(intr);
#ifdef DEBUG_INTR
    if (intr && intr == _rxIntr) {
        cli.print(F("@@ Negate RX INTR "));
        cli.printlnHex(intr, 2);
    }
    if (intr && intr == _txIntr) {
        cli.print(F("@@ Negate TX INTR "));
        cli.printlnHex(intr, 2);
    }
#endif
}

void I8251::setRxReady() {
    const auto old = _status;
    _status |= ST_RxRDY_bm;
    if ((_status ^ old) && _rxIntr)
        assertIntr(_rxIntr);
}

void I8251::resetRxReady() {
    const auto old = _status;
    _status &= ~ST_RxRDY_bm;
    if ((_status ^ old) && _rxIntr)
        negateIntr(_rxIntr);
}

void I8251::setTxReady() {
    const auto old = _status;
    _status |= (ST_TxRDY_bm | ST_TxEMPTY_bm);
    if ((_status ^ old) && _txIntr)
        assertIntr(_txIntr);
}

void I8251::resetTxReady() {
    const auto old = _status;
    _status &= ~(ST_TxRDY_bm | ST_TxEMPTY_bm);
    if ((_status ^ old) && _txIntr)
        negateIntr(_txIntr);
}

void I8251::loop() {
    if (!_enabled)
        return;
    if (rxEnabled()) {
        if (_stream.available() > 0 && rtsEnabled()) {
            _rxData = _stream.read();
            if (rxReady())
                _status |= ST_OE_bm;
            setRxReady();
#ifdef DEBUG_READ
            cli.print(F("@@ Recv "));
            cli.printHex(_rxData, 2);
            cli.print(' ');
            cli.printlnHex(_status, 2);
#endif
        }
    }
    // TODO: Implement flow command
    if (txEnabled()) {
        if (_stream.availableForWrite() > 0 && !txReady()) {
            _stream.write(_txData);
            setTxReady();
#ifdef DEBUG_WRITE
            cli.print(F("@@ Send "));
            cli.printHex(_txData, 2);
            cli.print(' ');
            cli.printlnHex(_status, 2);
#endif
        }
    }
}

void I8251::write(uint8_t data, uint16_t addr) {
    if (addr == _baseAddr) {
        _txData = data;
        resetTxReady();
#ifdef DEBUG_WRITE
        cli.print(F("@@ Write "));
        cli.printHex(data, 2);
        cli.print(' ');
        cli.printlnHex(_status, 2);
#endif
        return;
    }

    if (addr == _baseAddr + 1) {
        if (_state == STATE_COMMAND) {
#ifdef DEBUG_COMMAND
            cli.print(F("@@ Command "));
            cli.printlnHex(data, 2);
#endif
            if (data & CMD_IR_bm)
                reset();
            if (data & CMD_ER_bm)
                _status &= ~(ST_FE_bm | ST_OE_bm | ST_PE_bm);
            _command = data &
                       (CMD_RTS_bm | CMD_RxEN_bm | CMD_DTR_bm | CMD_TxEN_bm);
            if (txEnabled()) {
                setTxReady();
            } else {
                resetTxReady();
            }
            return;
        }
        if (_state == STATE_MODE) {
#ifdef DEBUG_COMMAND
            cli.print(F("@@ Mode "));
            cli.printlnHex(data, 2);
#endif
            _mode = data;
            _state = ((data & MODE_STOP_gm) == MODE_SYNC_gc) ? STATE_SYNC1
                                                             : STATE_COMMAND;
            return;
        }
        if (_state == STATE_SYNC1) {
#ifdef DEBUG_COMMAND
            cli.print(F("@@ Sync1 "));
            cli.printlnHex(data, 2);
#endif
            _state = STATE_SYNC2;
            return;
        }
        // STATE_SYNC2
#ifdef DEBUG_COMMAND
        cli.print(F("@@ Sync2 "));
        cli.printlnHex(data, 2);
#endif
        _state = STATE_COMMAND;
        return;
    }

    if (addr == _baseAddr + 2) {
        const auto intr = IntrName(data);
#ifdef DEBUG_INTR
        cli.print(F("@@ SET RX INTR "));
        cli.printlnHex(data, 2);
#endif
        if (intr == INTR_NONE) {
            negateIntr(_rxIntr);
        } else if (rxEnabled() && rxReady()) {
            assertIntr(intr);
        }
        _rxIntr = intr;
        return;
    }
    // addr == baseAddr + 3
    const auto intr = IntrName(data);
#ifdef DEBUG_INTR
    cli.print(F("@@ SET TX INTR "));
    cli.printlnHex(data, 2);
#endif
    if (intr == INTR_NONE) {
        negateIntr(_txIntr);
    } else if (txEnabled() && txReady()) {
        assertIntr(intr);
    }
    _txIntr = intr;
}

uint8_t I8251::read(uint16_t addr) {
    if (addr == _baseAddr) {
#ifdef DEBUG_READ
        const uint8_t prev_status = _status;
#endif
        resetRxReady();
#ifdef DEBUG_READ
        cli.print(F("@@ Read "));
        cli.printHex(_rxData, 2);
        cli.print(' ');
        cli.printHex(prev_status, 2);
        cli.print(F("->"));
        cli.printlnHex(_status, 2);
#endif
        return _rxData;
    }

    if (addr == _baseAddr + 1) {
#ifdef DEBUG_STATUS
        if (rxReady() || !txReady()) {
            cli.print(F("@@ Status "));
            cli.printlnHex(_status, 2);
        }
#endif
        return _status;
    }
    if (addr == _baseAddr + 2)
        return uint8_t(_rxIntr);
    // addr == _baseAddr + 3
    return uint8_t(_txIntr);
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:

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
    negateRxIntr();
    negateTxIntr();
    _rxVec = _txVec = INTR_NONE;
    _rxIntr = _txIntr = INTR_NONE;
}

void I8251::enable(bool enabled, uint16_t baseAddr) {
    _enabled = enabled;
    _baseAddr = baseAddr & ~3;
    reset();
#if defined(DEBUG_STATUS) || defined(DEBUG_COMMAND)
    cli.print("@@ Enable=");
    cli.printDec(_enabled);
    cli.print(" baseAddr=");
    cli.printlnHex(_baseAddr);
#endif
}

void I8251::assertRxIntr() {
    if (_rxIntr == INTR_NONE)
        return;
    _rxVec = _rxIntr;
    Pins.assertIntr(_rxVec);
#ifdef DEBUG_INTR
    cli.print(F("@@ Assert RX INTR "));
    cli.printlnHex(_rxIntr, 2);
#endif
}

void I8251::assertTxIntr() {
    if (_txIntr == INTR_NONE)
        return;
    _txVec = _txIntr;
    Pins.assertIntr(_txVec);
#ifdef DEBUG_INTR
    cli.print(F("@@ Assert TX INTR "));
    cli.printlnHex(intr, _txIntr);
#endif
}

void I8251::negateRxIntr() {
    if (_rxIntr == INTR_NONE)
        return;
    _rxVec = INTR_NONE;
#ifdef DEBUG_INTR
    cli.print(F("@@ Negate RX INTR "));
    cli.printlnHex(_rxIntr, 2);
#endif
    if (_txVec != _rxIntr)
        Pins.negateIntr(_rxIntr);
}

void I8251::negateTxIntr() {
    if (_txIntr == INTR_NONE)
        return;
    _txVec = INTR_NONE;
#ifdef DEBUG_INTR
    cli.print(F("@@ Negate TX INTR "));
    cli.printlnHex(_txIntr, 2);
#endif
    if (_rxVec != _txIntr)
        Pins.negateIntr(_txIntr);
}

void I8251::setRxReady() {
    const auto old = _status;
    _status |= ST_RxRDY_bm;
    if ((_status ^ old) && _rxIntr)
        assertRxIntr();
}

void I8251::resetRxReady() {
    const auto old = _status;
    _status &= ~ST_RxRDY_bm;
    if ((_status ^ old) && _rxIntr)
        negateRxIntr();
}

void I8251::setTxReady() {
    const auto old = _status;
    _status |= (ST_TxRDY_bm | ST_TxEMPTY_bm);
    if ((_status ^ old) && _txIntr)
        assertTxIntr();
}

void I8251::resetTxReady() {
    const auto old = _status;
    _status &= ~(ST_TxRDY_bm | ST_TxEMPTY_bm);
    if ((_status ^ old) && _txIntr)
        negateTxIntr();
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
        if (intr == INTR_NONE && _rxVec)
            negateRxIntr();  // negate old _rxVec
        _rxIntr = intr;
        if (intr && rxEnabled() && rxReady())
            assertRxIntr();
        return;
    }
    // addr == baseAddr + 3
    const auto intr = IntrName(data);
#ifdef DEBUG_INTR
    cli.print(F("@@ SET TX INTR "));
    cli.printlnHex(data, 2);
#endif
    if (intr == INTR_NONE && _txVec)
        negateTxIntr();  // negate old _txVec
    _txIntr = intr;
    if (intr && txEnabled() && txReady())
        assertTxIntr();
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

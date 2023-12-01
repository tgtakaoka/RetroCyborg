#ifndef __TLCS90_UART_HANDLER_H__
#define __TLCS90_UART_HANDLER_H__

#include <Arduino.h>
#include <stdint.h>

#include "config.h"

class Tlcs90UartHandler {
public:
    Tlcs90UartHandler(Stream &stream) : _stream(stream), _enabled(false) {
        digitalWriteFast(PIN_RXD, HIGH);
        pinMode(PIN_RXD, OUTPUT);
        pinMode(PIN_TXD, INPUT_PULLUP);
        reset();
    }

    /** @param baudrate in BCD */
    void reset(uint16_t baudrate = 0x9600) {
        // TLCS90 UART: assuming XTAL is fc=9.8304MHz
        // bitrate=fc/4/Tn/SC/16
        // TRUN_BRATE=11, T4 clock, 9600bps
        // SCMOD_SC=11, 1/2 division
        switch (baudrate) {
        case 0x9600:
        default:  // 9600bps
            _baudrate = 9600;
            _pre_divider = 4 * 4 * 2;
            _divider = 16;
            break;
        }
        _prescaler = _pre_divider;
        _rx.bit = 0;
        _tx.bit = 0;
    }

    void loop() {
        if (_enabled && --_prescaler == 0) {
            _prescaler = _pre_divider;
            txloop(_tx);
            rxloop(_rx);
        }
    }

    /** @param baudrate in BCD */
    void enable(bool enabled, uint16_t baudrate = 0x9600) {
        _enabled = enabled;
        reset(baudrate);
    }

    /** @return base address of SIO */
    uint16_t baseAddr() const { return 0xF0; }

    /** @return baudrate in decimal */
    uint16_t baudrate() const { return _baudrate; }

private:
    Stream &_stream;
    bool _enabled;
    uint16_t _baudrate;
    uint16_t _divider;
    uint16_t _pre_divider;
    uint16_t _prescaler;
    struct Transmitter {
        uint8_t bit;
        uint8_t data;
        uint16_t delay;
    } _tx;
    struct Receiver {
        uint8_t bit;
        uint8_t data;
        uint16_t delay;
    } _rx;

    void txloop(Transmitter &tx) {
        if (tx.bit == 0) {
            if (_stream.available()) {
                tx.bit = 10;  // start bit + data bits + stop bit
                tx.data = _stream.read();
                tx.delay = _divider;
                digitalWriteFast(PIN_RXD, LOW);  // start bit
            }
        } else {
            if (--tx.delay == 0) {
                digitalWriteFast(PIN_RXD, tx.data & 1);
                tx.data >>= 1;
                tx.data |= 0x80;  // stop and mark bits
                tx.delay = _divider;
                if (--tx.bit == 0)
                    digitalWriteFast(PIN_RXD, HIGH);  // output mark/idle
            }
        }
    }

    void rxloop(Receiver &rx) {
        if (rx.bit == 0) {
            if (digitalReadFast(PIN_TXD) == LOW) {
                rx.bit = 9;
                rx.data = 0;
                rx.delay = _divider + (_divider >> 1);
            }
        } else {
            if (--rx.delay == 0) {
                rx.data >>= 1;
                if (digitalReadFast(PIN_TXD) == HIGH)
                    rx.data |= 0x80;
                rx.delay = _divider;
                if (--rx.bit == 1) {
                    _stream.write(rx.data);
                    // stop bit will be ignored.
                }
            }
        }
    }
};

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:

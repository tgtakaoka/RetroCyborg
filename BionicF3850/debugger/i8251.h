#ifndef __I8251_H__
#define __I8251_H__

#include <Arduino.h>
#include <stdint.h>

/**
 * i8251 USART device emulator.
 *
 * Support only asynchronous mode. Stop bit and bit length, clock aren't
 * emulated. Interrupt support (via TxEMPTY, RxRDY).
 */
class I8251 {
public:
    I8251(Stream &stream);

    void reset();
    bool isSelected(uint16_t addr) const {
        // addr+0: USART data
        // addr+1: USART control/status
        // addr+2: USART Rx interrupt vector
        // addr+3: USART Tx interrupt vector
        return _enabled && (addr & ~3) == _baseAddr;
    }
    void write(uint8_t data, uint16_t addr);
    uint8_t read(uint16_t addr);
    void loop();
    void enable(bool enabled, uint16_t baseAddr);
    uint16_t baseAddr() const { return _baseAddr; }
    uint16_t intrVec() const { return _rxVec ? _rxVec : _txVec; }

private:
    Stream &_stream;
    bool _enabled;

    enum ModeState : uint8_t {
        STATE_MODE = 0,
        STATE_SYNC1 = 1,
        STATE_SYNC2 = 2,
        STATE_COMMAND = 3,
    };

    ModeState _state;
    uint8_t _mode;
    uint8_t _command;
    uint8_t _status;
    uint8_t _txData;
    uint8_t _rxData;
    uint8_t _rxIntr;
    uint8_t _txIntr;
    uint16_t _baseAddr;
    uint8_t _rxVec;
    uint8_t _txVec;

    void assertRxIntr();
    void negateRxIntr();
    void assertTxIntr();
    void negateTxIntr();
    void assertIntreq() const;
    void negateIntreq() const;
    bool rxEnabled() const { return _command & CMD_RxEN_bm; }
    bool txEnabled() const { return _command & CMD_TxEN_bm; }
    bool rxReady() const { return _status & ST_RxRDY_bm; }
    bool txReady() const { return _status & ST_TxRDY_bm; }
    bool rtsEnabled() const { return _command & CMD_RTS_bm; }
    void setRxReady();
    void resetRxReady();
    void setTxReady();
    void resetTxReady();

    // Bit Definition of mode regisrer
    static constexpr int MODE_STOP_gp = 6;
    static constexpr uint8_t MODE_STOP_gm = 0xC0;
    static constexpr uint8_t MODE_SYNC_gc = (0 << MODE_STOP_gp);
    static constexpr uint8_t MODE_STOP1_gc = (1 << MODE_STOP_gp);
    static constexpr uint8_t MODE_STOP15_gc = (2 << MODE_STOP_gp);
    static constexpr uint8_t MODE_STOP2_gc = (3 << MODE_STOP_gp);
    static constexpr uint8_t MODE_EVEN_bm = 0x20;
    static constexpr uint8_t MODE_PARITY_bm = 0x10;
    static constexpr int MODE_LEN_gp = 2;
    static constexpr uint8_t MODE_LEN_gm = 0x0C;
    static constexpr uint8_t MODE_LEN5_gc = (0 << MODE_LEN_gp);
    static constexpr uint8_t MODE_LEN6_gc = (1 << MODE_LEN_gp);
    static constexpr uint8_t MODE_LEN7_gc = (2 << MODE_LEN_gp);
    static constexpr uint8_t MODE_LEN8_gc = (3 << MODE_LEN_gp);
    static constexpr int MODE_BAUD_gp = 0;
    static constexpr uint8_t MODE_BAUD_gm = 0x03;
    static constexpr uint8_t MODE_BAUD_X1 = (1 << MODE_BAUD_gp);
    static constexpr uint8_t MODE_BAUD_X16 = (2 << MODE_BAUD_gp);
    static constexpr uint8_t MODE_BAUD_X64 = (3 << MODE_BAUD_gp);
    // Bit Definition of command register
    static constexpr uint8_t CMD_EH_bm = 0x80;    // Enter hunt mode
    static constexpr uint8_t CMD_IR_bm = 0x40;    // Internal Reset
    static constexpr uint8_t CMD_RTS_bm = 0x20;   // Request To Send
    static constexpr uint8_t CMD_ER_bm = 0x10;    // Error Reset
    static constexpr uint8_t CMD_SBRK_bm = 0x08;  // Send Break
    static constexpr uint8_t CMD_RxEN_bm = 0x04;  // Receive Enable
    static constexpr uint8_t CMD_DTR_bm = 0x02;   // Data Terminal Ready
    static constexpr uint8_t CMD_TxEN_bm = 0x01;  // Transmit enable

    // Bit definition of status register
    static constexpr uint8_t ST_DSR_bm = 0x80;      // Data Set Readt
    static constexpr uint8_t ST_BRK_bm = 0x40;      // BREAK detected
    static constexpr uint8_t ST_FE_bm = 0x20;       // Framing Error
    static constexpr uint8_t ST_OE_bm = 0x10;       // Iverrun Error
    static constexpr uint8_t ST_PE_bm = 0x08;       // Parity Error
    static constexpr uint8_t ST_TxEMPTY_bm = 0x04;  // Transmitter empty
    static constexpr uint8_t ST_RxRDY_bm = 0x02;    // Receiver ready
    static constexpr uint8_t ST_TxRDY_bm = 0x01;    // Transmitter ready
};

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:

#ifndef __MC6850_H__
#define __MC6850_H__

#include <stdint.h>

class Mc6850 {
  public:
    Mc6850(uint16_t baseAddr);

    bool isSelected(uint16_t addr) const {
      return addr == _baseAddr || --addr == _baseAddr;
    }

    void write(uint8_t data, uint16_t addr);
    uint8_t read(uint16_t addr);
    void loop();

  private:
    const uint16_t _baseAddr;
    const uint8_t _rxIrq;
    const uint8_t _txIrq;
    uint8_t _control;
    uint8_t _status;
    uint8_t _readFlags;
    uint8_t _nextFlags;
    uint8_t _sendingData;
    uint8_t _receivedData;

    // Bit Definition of control register
    // CDSx, WSBx, TCB=3 are ignored.
    static constexpr uint8_t CDS0 = 0x01; // Counter Divider Select bit 0
    static constexpr uint8_t CDS1 = 0x02; // Counter Divider Select bit 1
    static constexpr uint8_t WSB0 = 0x04; // Word Select bit 0
    static constexpr uint8_t WSB1 = 0x08; // Word Select bit 1
    static constexpr uint8_t WSB2 = 0x10; // Word Select bit 2
    static constexpr uint8_t TCB0 = 0x20; // Transmit Control bit 0
    static constexpr uint8_t TCB1 = 0x40; // Transmit Control bit 1
    static constexpr uint8_t RIEB = 0x80; // Receive Interrupt Enable bit

    bool txIntEnabled() const {
      return (_control & (TCB0 | TCB1)) == TCB0;
    }
    bool rxIntEnabled() const {
      return (_control & RIEB) != 0;
    }
    void assertIrq(uint8_t irq);
    void negateIrq(uint8_t irq);

    // Bit definition of status register
    // DCD, CTS, FERR, PERR are always zero.
    static constexpr uint8_t RDRF = 0x01; // Receive Data Register Full
    static constexpr uint8_t TDRE = 0x02; // Transmit Data Register Empty
    static constexpr uint8_t DCD  = 0x04; // Data Carieer Detect
    static constexpr uint8_t CTS  = 0x08; // Clear to Send
    static constexpr uint8_t FERR = 0x10; // Framing Error
    static constexpr uint8_t OVRN = 0x20; // Receiver Overrun
    static constexpr uint8_t PEER = 0x40; // Parity Error
    static constexpr uint8_t IRQF = 0x80; // Interrupt Request
};

#endif

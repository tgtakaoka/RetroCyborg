#include "regs_mc6805.h"
#include "debugger.h"
#include "mems_mc6805.h"
#include "pins_mc6805.h"
#include "string_util.h"

namespace debugger {
namespace mc6805 {

void RegsMc6805::print() const {
    // clang-format off
    static char buffer[] = {
        'P', 'C', '=', 0, 0, 0, 0, ' ', // PC=3
        'S', 'P', '=', 0, 0, 0, 0, ' ', // SP=11
        'X', '=', 0, 0, ' ',            // X=18
        'A', '=', 0, 0, ' ',            // A=23
        'C', 'C', '=', 0, 0, 0, 0, 0,   // CC=29
        0,                              // EOS
    };
    // clang-format on
    outHex16(buffer + 3, _pc);
    outHex16(buffer + 11, _sp);
    outHex8(buffer + 18, _x);
    outHex8(buffer + 23, _a);
    auto *p = buffer + 29;
    *p++ = bit1(_cc & 0x10, 'H');
    *p++ = bit1(_cc & 0x08, 'I');
    *p++ = bit1(_cc & 0x04, 'N');
    *p++ = bit1(_cc & 0x02, 'Z');
    *p++ = bit1(_cc & 0x01, 'C');
    *p = 0;
    cli.println(buffer);
}

void RegsMc6805::save() {
    const uint8_t SWI = 0x83;  // 1:N:w:W:W:W:W:V:v:A
    _pins->injectReads(&SWI, sizeof(SWI));
    uint8_t context[5];
    _pins->captureWrites(context, sizeof(context), &_sp);
    // Capturing writes to stack in little endian order.
    _pc = le16(context) - 1;  //  offset SWI instruction.
    _x = context[2];
    _a = context[3];
    _cc = context[4];
    context[0] = hi(_pc);
    context[1] = lo(_pc);
    _pins->injectReads(context, 2);
    _pins->suspend();
}

void RegsMc6805::restore() {
    // Store registers into stack on internal RAM.
    _sp -= 4;
    internal_write(_sp++, _cc);
    internal_write(_sp++, _a);
    internal_write(_sp++, _x);
    internal_write(_sp++, hi(_pc));
    internal_write(_sp, lo(_pc));
    // Restore registers
    const uint8_t RTI = 0x80;  // 1:N:n:n:n:n:n:n:N
    _pins->injectReads(&RTI, sizeof(RTI));
    _pins->suspend();
}

bool RegsMc6805::saveContext(const Signals *frame) {
    // Machine context were pushed in the following order; PCL, PCH, X, A, CC
    const auto pcl = frame;
    const auto pch = frame->next();
    if (pcl->write() && pcl->write()) {
        _pc = uint16(pch->data, pcl->data);
        _sp = frame->addr;
        const auto x = frame->next(2);
        const auto a = frame->next(3);
        const auto cc = frame->next(4);
        if (x->write() && a->write() && cc->write()) {
            _x = x->data;
            _a = a->data;
            _cc = cc->data;
            return true;
        }
    }
    return false;
}

void RegsMc6805::helpRegisters() const {
    cli.println("?Reg: PC X A CC");
}

constexpr const char *REGS8[] = {
        "A",   // 1
        "X",   // 2
        "CC",  // 3
};

constexpr const char *REGS16[] = {
        "PC",  // 4
};

const Regs::RegList *RegsMc6805::listRegisters(uint8_t n) const {
    static constexpr RegList REG_8{REGS8, 3, 1, UINT8_MAX};
    static RegList REG_16{REGS16, 1, 4, _mems->maxAddr()};
    return n == 0 ? &REG_8 : (n == 1 ? &REG_16 : nullptr);
}

void RegsMc6805::setRegister(uint8_t reg, uint32_t value) {
    switch (reg) {
    case 4:
        _pc = value;
        break;
    case 2:
        _x = value;
        break;
    case 1:
        _a = value;
        break;
    case 3:
        _cc = value;
        break;
    }
}

uint8_t RegsMc6805::internal_read(uint16_t addr) const {
    uint8_t LDA[2];
    LDA[0] = 0xB6;  // LDA addr ; 1:2:D
    LDA[1] = addr;
    _pins->injectReads(LDA, sizeof(LDA), 3);
    static const uint8_t STA_0F00[] = {
            0xC7, 0x0F, 0x00,  // STA $0F00 ; 1:2:3:n:E
    };
    _pins->injectReads(STA_0F00, sizeof(STA_0F00));
    uint8_t data;
    _pins->captureWrites(&data, sizeof(data));
    _pins->suspend();
    return data;
}

void RegsMc6805::internal_write(uint16_t addr, uint8_t data) const {
    uint8_t LDA_STA[2 + 3];
    LDA_STA[0] = 0xA6;  // LDA #val ; 1:2
    LDA_STA[1] = data;
    LDA_STA[2] = 0xC7;  // STA addr ; 1:2:3:n:E
    LDA_STA[3] = hi(addr);
    LDA_STA[4] = lo(addr);
    _pins->injectReads(LDA_STA, sizeof(LDA_STA));
    _pins->suspend();
}

}  // namespace mc6805
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:

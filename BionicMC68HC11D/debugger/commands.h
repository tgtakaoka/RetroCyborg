#ifndef __COMMANDS_H__
#define __COMMANDS_H__

class Commands {
public:
    void begin();
    void loop();
    void exec(char c);
    void halt(bool show = false);
    bool isRunning() const { return _target == RUN; }

private:
    enum {
        HALT,
        STEP,
        RUN,
    } _target = HALT;
    bool _showRegs;
};

extern Commands Commands;

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:

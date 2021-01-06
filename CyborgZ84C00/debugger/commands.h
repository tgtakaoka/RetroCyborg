/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __COMMANDS_H__
#define __COMMANDS_H__

class Commands {
public:
    void begin();
    void loop();
    void exec(char c);
    bool isRunning() const { return _target == RUN; }

private:
    enum { HALT, STEP, RUN } _target = HALT;
};

extern Commands Commands;

#endif

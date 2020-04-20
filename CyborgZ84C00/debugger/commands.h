/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __commands_h__
#define __commands_h__

class Commands {

public:
  void begin();
  void loop();
  bool exec(char c);
  bool isRunning() const { return _target == RUN; }

private:
  enum {
    HALT, STEP, RUN
  } _target = HALT;
};

extern Commands Commands;

#endif

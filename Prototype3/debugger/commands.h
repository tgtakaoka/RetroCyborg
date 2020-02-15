/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __commands_h__
#define __commands_h__

class Commands {

public:
  void begin();
  void loop();
  bool exec(char c);
  bool isRunning() const { return _target == RUN; }
  void halt(bool show);

private:
  enum {
    HALT, STEP, RUN, PROMPT,
  } _target = HALT;
};

extern Commands Commands;

#endif

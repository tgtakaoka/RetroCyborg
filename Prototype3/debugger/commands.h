#ifndef __commands_h__
#define __commands_h__

class Commands {

  public:
    void loop();
    void exec(char c);

  private:
    enum {
      HALT, STEP, RUN
    } _target = HALT;
};

extern Commands Commands;

#endif

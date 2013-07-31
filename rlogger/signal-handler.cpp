#include "common.h"
#include <iostream>
int rlog::Interrupt::interrupted_ = 0;

extern "C" void rlog::RloggerSignalHandler(int signal_value) {
    rlog::Interrupt::interrupted_ = 1;
}

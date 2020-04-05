#include "common.h"
#include <sys/types.h>

typedef enum{
    TEXT_E,
    SIGNAL_E
}inputType_e;


void writer(const char* fifoName, pid_t readerPid);
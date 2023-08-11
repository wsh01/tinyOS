#ifndef THOOKS_H
#define THOOKS_H

#include "tinyOS.h"

void tHooksCpuIdle (void);
void tHooksSysTicks (void);
void tHooksTaskSwitch (tTask * from, tTask * to);
void tHooksTaskInit (tTask * task);

#endif



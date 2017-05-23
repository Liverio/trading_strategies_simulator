#ifndef REPORT_H
#define REPORT_H

#include "types.h"
#include "money_management.h"

void report(char *ticker, tp_trade *trades, int trades_no, tp_sample *sample, float stop_loss, float take_profit);

#endif

#ifndef MONEY_MANAGEMENT_H
#define MONEY_MANAGEMENT_H

#include "data_adquisition.h"
#include "types.h"

#define WARRANTY 0.01f	//#define WARRANTY 0.05f	//#define WARRANTY 0.15f
#define MARGIN_THRESHOLD 0.05f
#define MAX_LEVERAGE 6.0f	//100.0f
#define INITIAL_CAPITAL 100000.0f

#define CASHOUT_THRESHOLD 200000.0f

#define DO_NOT_OVERLAP_LONG_SHORT 0

typedef struct {
        char date[11];
        char time[5];
        int day_no;
        int month_no;
        int year_no;
        int time_slot;
} tp_sample_MM;

void create_samples_MM(tp_sample *sample, int days_no, tp_sample_MM *sample_MM);
char optimize_money_management(tp_sample_MM *sample_MM, tp_trade *trades_IBEX, tp_trade *trades_SP500, int trades_no, float *trade_size);
void report_money_management(tp_sample_MM *sample_MM, char *ticker, tp_trade *trades);

#endif

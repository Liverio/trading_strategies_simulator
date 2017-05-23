#ifndef DATA_ADQUISITION_H
#define DATA_ADQUISITION_H

#define MAX_SAMPLES 10000000

typedef struct {
        char date[11];
        char time[5];
        float open;
        float close;
		float max;
        float min;
        int day_no;
        int month_no;
        int year_no;
        int time_slot;
} tp_sample;

static char initial_date[5] = "2000";

void read_data(char *ticker, tp_sample *sample);

#endif

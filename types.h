#ifndef TYPES_H
#define TYPES_H

typedef enum {LONG, SHORT} tp_direction;

typedef struct {
        int sample_in;
        int sample_out;
        char date_in[9];
        char time_in[5];
		char date_out[9];
        char time_out[5];
        tp_direction direction;
        float outcome;
        float price_in;
} tp_trade;

#endif

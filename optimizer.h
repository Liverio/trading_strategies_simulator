#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "data_adquisition.h"
#include "types.h"

typedef struct {
        float stop_loss;
        float take_profit;
		float profit;
        float deviation;
} tp_best_setup;

//#define PROFIT_WEIGHT	0.75f
//#define VARIANCE_WEIGHT 0.25f
#define PROFIT_WEIGHT	0.50f
#define VARIANCE_WEIGHT 0.50f

#define MIN_SL 0.001f
#define MAX_SL 0.01f
#define MIN_TP 0.003f
#define MAX_TP 0.02f

/*
#define MIN_SL 0.001f
#define MAX_SL 0.02f
#define MIN_TP 0.003f
#define MAX_TP 0.05f
*/


void optimize(char *ticker, tp_sample *sample, float *stop_loss, float *take_profit);

#endif

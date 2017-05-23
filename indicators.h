#ifndef INDICATORS_H
#define INDICATORS_H

#include "types.h"
#include "data_adquisition.h"

#define SECONDARY_TF 15
#define SMA_PERIOD 20
#define EMA_PERIOD 9
#define STOCH_PERIODS 14
//#define FIRST_SAMPLE SMA_PERIOD
#define FIRST_SAMPLE (SMA_PERIOD * SECONDARY_TF)

void calculate_SMA(tp_sample *sample, int periods, int tf, float *destination);
void calculate_EMA(tp_sample *sample, int periods, int tf, float *destination);
void calculate_RSI(tp_sample *sample, int periods, int tf, float *destination);
void calculate_ADX(tp_sample *sample, int periods);
float WMA(tp_sample *sample, int sample_no, int periods);
void calculate_HMA(tp_sample *sample, int sample_no, int periods, float *HMA);

#endif

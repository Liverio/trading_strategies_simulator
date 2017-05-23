#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "types.h"
#include "data_adquisition.h"

//#define SPREAD 2
//#define RAKE (SPREAD / sample[trades[trade_no].sample_in].open)
//#define RAKE 0.00003f	// Futures DEGIRO
//#define RAKE 0.000072f	// Futures Interactive Brokers
#define RAKE 0.0f
#define SLIPPAGE 0.0f
//#define SLIPPAGE 2.0 / (10000 / (sample[num_sample_signal + 1].open))	//#define SLIPPAGE 1.0 / (10000 / (sample[num_sample_signal + 1].open))

int simulate(tp_sample *sample, float stop_loss, float take_profit, float *outcome, tp_trade *trades);
void simulate_trade(tp_sample *sample, int num_sample_signal, tp_direction direction, float stop_loss, float take_profit, int *sample_closed, float *outcome);

#endif

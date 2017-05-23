#include "simulator.h"
#include "signals.h"
#include "indicators.h"

extern int SAMPLES;
extern float *SMA, *SMA_tf5, *SMA_tf15;
extern float *EMA, *EMA_tf5, *EMA_tf15;
extern float *RSI, *RSI_tf5, *RSI_tf15;
extern float *SK_tf1, *SK_tf15;

#define ONLY_INTRADAY 1
#define USE_TAKE_PROFIT 1
#define USE_TRAILING_STOP 0
#define MOVE_STOP_ENTRY_FACTOR 0.20f	//#define MOVE_STOP_ENTRY_FACTOR 0.20f
#define TRALING_STOP_FACTOR 0.33f		//#define TRALING_STOP_FACTOR 0.30f


int simulate(tp_sample *sample, float stop_loss, float take_profit, float *outcome, tp_trade *trades){
	int i;
	int initial_sample, sample_found;
	tp_direction direction;
	int sample_closed;
	int trade_no = 0;
	float trade_outcome;
	
	initial_sample = FIRST_SAMPLE;
	*outcome = 0;

	while (find_signal(initial_sample, &sample_found, &direction)){
		simulate_trade(sample, sample_found, direction, stop_loss, take_profit, &sample_closed, &trade_outcome);
		
		trades[trade_no].outcome = trade_outcome - RAKE;
		*outcome += trades[trade_no].outcome;		
		trades[trade_no].direction  = direction;
		trades[trade_no].sample_in  = sample_found + 1;
		trades[trade_no].sample_out = sample_closed;
		strcpy(trades[trade_no].date_in, sample[sample_found + 1].date);
		strcpy(trades[trade_no].time_in, sample[sample_found + 1].time);
		trades[trade_no].sample_out = sample_closed;
		strcpy(trades[trade_no].date_out, sample[sample_closed].date);
		strcpy(trades[trade_no].time_out, sample[sample_closed].time);
		// Needed to calculate the future value
		trades[trade_no].price_in = sample[sample_found + 1].open;
		trade_no++;
					
		initial_sample = sample_found;
	}
	
	// Return number of trades
	return trade_no;
}

void simulate_trade(tp_sample *sample, int num_sample_signal, tp_direction direction, float stop_loss, float take_profit, int *sample_closed, float *outcome){
	int i, j;
	// Signal considers the closing price of 'num_sample_signal'. Trade must be opened the next candle
	float open_price = direction == LONG ? sample[num_sample_signal + 1].open + SLIPPAGE : sample[num_sample_signal + 1].open - SLIPPAGE;
	float stop_loss_price = direction == LONG ? (1 - stop_loss) * sample[num_sample_signal].close : (1 + stop_loss) * sample[num_sample_signal].close;
	
	// Trailing stop
	float current_max = open_price, current_min = open_price;
	float stop_profit;

	for (i = num_sample_signal + 1; i < SAMPLES - 1; i++){
		// LONG
		if (direction == LONG){				
			// Stop reached
			if (sample[i].min <= stop_loss_price){
				*sample_closed = i;
				// Gap
				if (sample[i].open <= stop_loss_price)
					*outcome = (sample[i].open - SLIPPAGE - open_price) / open_price;
				else
					*outcome = (stop_loss_price - SLIPPAGE - open_price) / open_price;
				return ;
			}
			#if USE_TAKE_PROFIT
				// Profit reached
				if (sample[i].max >= (1 + take_profit) * open_price){
					*sample_closed = i;
					// Gap
					if (sample[i].open >= (1 + take_profit) * open_price)					
						*outcome = (sample[i].open - open_price) / open_price;
					else
						*outcome = take_profit;
					return ;
				}
			#endif
			
			#if USE_TRAILING_STOP
				if (sample[i].max > current_max)
					current_max = sample[i].max;

				if (current_max >= (1 + MOVE_STOP_ENTRY_FACTOR * take_profit) * open_price)
					stop_loss_price = open_price + TRALING_STOP_FACTOR * (current_max - open_price);
					//stop_loss_price = current_max - TRALING_STOP_FACTOR * ATR(sample, i);
			#endif
			
			#if ONLY_INTRADAY			
				// End of day
				if (strcmp(sample[i].date, sample[i+1].date)){
					*sample_closed = i;
					*outcome = (sample[i].close - open_price) / open_price;
					return ;
				}
			#endif
		}
		// SHORT
		else{	
			// Stop reached
			if (sample[i].max >= stop_loss_price){
				*sample_closed = i;
				// Gap
				if (sample[i].open >= stop_loss_price)
					*outcome = (open_price - (sample[i].open + SLIPPAGE)) / open_price;
				else
					*outcome = (open_price - (stop_loss_price + SLIPPAGE)) / open_price;
				return ;
			}
			
			#if USE_TAKE_PROFIT
				// Profit reached
				if (sample[i].min <= (1 - take_profit) * open_price){
					*sample_closed = i;
					// Gap
					if (sample[i].open <= (1 - take_profit) * open_price)					
						*outcome = -(sample[i].open - open_price) / open_price;
					else
						*outcome = take_profit;						
					return ;
				}
			#endif

			#if USE_TRAILING_STOP
				if (sample[i].min < current_min)
					current_min = sample[i].min;
				
				if (current_min <= (1 - MOVE_STOP_ENTRY_FACTOR * take_profit) * open_price)
					stop_loss_price = open_price - TRALING_STOP_FACTOR * (open_price - current_min);
					//stop_loss_price = current_min + TRALING_STOP_FACTOR * ATR(sample, i);
			#endif
			
			#if ONLY_INTRADAY
				// End of day
				if (strcmp(sample[i].date, sample[i+1].date)){
					*sample_closed = i;
					*outcome = -(sample[i].close - open_price) / open_price;
					return ;
				}
			#endif
		}
	}
	
	// Trade still open. Consider it was closed at the last sample
	*sample_closed = SAMPLES - 1;
	if (direction == LONG)
		*outcome = (sample[SAMPLES - 1].close - open_price) / open_price;
	else
		*outcome = -(sample[SAMPLES - 1].close - open_price) / open_price;
		
	return ;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "optimizer.h"
#include "simulator.h"
#include "indicators.h"

extern float *SMA;
extern float *EMA;
extern float *RSI;
extern float *ADX;
extern int SAMPLES;

#define DUMP_OPTIMIZATION_MAP 1
#define BEST_SETUPS_NO 10

void optimize(char *ticker, tp_sample *sample, float *stop_loss, float *take_profit){
	float current_sl, current_tp;
	int sample_closed;
	int initial_sample, sample_found;
	tp_direction direction;
	float outcome;
	float max_profit = 0;
	float max_deviation = 0;
	int i, j;
	float *dayly_outcome, *monthly_outcome;
	int days_no = sample[SAMPLES-1].day_no + 1;
	int months_no = sample[SAMPLES-1].month_no + 1;
	float trade_outcome;
	// Weighted profit and std dev optimization
	float profit[18*35];	//float profit[(MAX_TP - MIN_TP) / *(MAX_SL/MIN_SL)];
	float std_dev[18*35];	//float std_dev[(MAX_TP/MIN_TP)*(MAX_SL/MIN_SL)];
	
	dayly_outcome = (float *)malloc(days_no * sizeof(float));
	monthly_outcome = (float *)malloc(months_no * sizeof(float));
		
	#if DUMP_OPTIMIZATION_MAP
		// Heat map to analyze optimization outcome
		FILE *f_optimization_map_profit, *f_optimization_map_variance;
		char path_optimization_map_profit[100], path_optimization_map_variance[100];
		
		sprintf(path_optimization_map_profit, "./output_data/%s/optimization_map_profit.txt", ticker);
		sprintf(path_optimization_map_variance, "./output_data/%s/optimization_map_variance.txt", ticker);
	    f_optimization_map_profit = fopen(path_optimization_map_profit, "w");
	    f_optimization_map_variance = fopen(path_optimization_map_variance, "w");
	    // Cols
		for (current_tp = MIN_TP; current_tp <= MAX_TP; current_tp += 0.0005){
			fprintf(f_optimization_map_profit, "\t%.4f", current_tp);
			fprintf(f_optimization_map_variance, "\t%.4f", current_tp);
		}    	
    #endif
	
	// OPTIMIZATION
	int setup_no = 0;
	for (current_sl = MIN_SL; current_sl <= MAX_SL; current_sl += 0.0005){
		#if DUMP_OPTIMIZATION_MAP
			// Rows
	    	fprintf(f_optimization_map_profit, "\n%.4f", current_sl);
	    	fprintf(f_optimization_map_variance, "\n%.4f", current_sl);
    	#endif
    	for (current_tp = MIN_TP; current_tp <= MAX_TP; current_tp += 0.0005){
			initial_sample = FIRST_SAMPLE;
			// Reset outcomes
			for (i = 0; i < days_no; i++)
				dayly_outcome[i] = 0;
			for (i = 0; i < months_no; i++)
				monthly_outcome[i] = 0;

			while (find_signal(initial_sample, &sample_found, &direction)){
				simulate_trade(sample, sample_found, direction, current_sl, current_tp, &sample_closed, &trade_outcome);
					
				// Add outcome to the day and month the trade was closed
				dayly_outcome[sample[sample_closed].day_no] += 100*trade_outcome;
				monthly_outcome[sample[sample_closed].month_no] += 100*trade_outcome;
					
				initial_sample = sample_found;
			}
			
			// Total outcome
			outcome = 0;
			for (i = 0; i < months_no; i++)
				outcome += monthly_outcome[i];
			
			// Store profit
			profit[setup_no] = outcome;
			
			if (outcome > max_profit)
				max_profit = outcome;
			
			// Variance
			float deviation = 0;
			float ev;
			
			// Only setups which lead to a positive outcome
			if (outcome > 0){
				ev = outcome / months_no;
				for (i = 0; i < months_no; i++){
					deviation += monthly_outcome[i] > ev ? (monthly_outcome[i] - ev) / ev : (ev - monthly_outcome[i]) / ev;
				}
				deviation /= months_no;
				
				// Store std_dev
				std_dev[setup_no] = deviation;
			}
			
			#if DUMP_OPTIMIZATION_MAP
				fprintf(f_optimization_map_profit, "\t%.1f", outcome);
				if (outcome > 0)
					fprintf(f_optimization_map_variance, "\t%.1f", deviation);
				else
					fprintf(f_optimization_map_variance, "\t-");
			#endif
			
			setup_no++;
		}
	}
	
	// *** Select the best scored setup ***
	tp_best_setup best_setup[BEST_SETUPS_NO];
	
	// Select BEST_SETUPS_NO best (profit) setups
	int last_best_index;
	for (i = 0; i < BEST_SETUPS_NO; i++){
		max_profit = 0;
		for (j = 0; j < 18*35; j++){
			if (profit[j] > max_profit){
				max_profit = profit[j];
				best_setup[i].profit = profit[j];
				best_setup[i].deviation = std_dev[j];
				best_setup[i].stop_loss = MIN_SL + (j / 35) * 0.0005;
				best_setup[i].take_profit = MIN_TP + (j % 35) * 0.0005;
				last_best_index = j;
			}
		}
		profit[last_best_index] = 0;
	}
	
	// Find largest deviation
	max_profit = best_setup[0].profit;
	for (i = 0; i < BEST_SETUPS_NO; i++){
		if (best_setup[i].deviation > max_deviation)
			max_deviation = best_setup[i].deviation;
	}
	
	// Normalize profit & std_dev outcomes
	for (i = 0; i < BEST_SETUPS_NO; i++){
		best_setup[i].profit = best_setup[i].profit / max_profit;
		best_setup[i].deviation = best_setup[i].deviation / max_deviation;
	}

	// Search the best scored setup
	float current_score, best_score = 0;
	for (i = 0; i < BEST_SETUPS_NO; i++){
		current_score = PROFIT_WEIGHT * best_setup[i].profit + VARIANCE_WEIGHT * (1 - best_setup[i].deviation);		
		if (current_score > best_score){
			best_score = current_score;
			*stop_loss = best_setup[i].stop_loss;
			*take_profit = best_setup[i].take_profit;
		}
	}

	#if DUMP_OPTIMIZATION_MAP
		fclose(f_optimization_map_profit);
		fclose(f_optimization_map_variance);
	#endif
	
	return ;
}

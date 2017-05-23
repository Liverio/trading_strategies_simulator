#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "data_adquisition.h"
#include "types.h"
#include "indicators.h"
#include "signals.h"
#include "simulator.h"
#include "optimizer.h"
#include "money_management.h"
#include "report.h"

extern int SAMPLES;

// Full data
tp_sample *sample;
tp_sample_MM *sample_MM;

FILE *f_tickers;

float *SMA, *SMA_tf5, *SMA_tf15;
float *EMA, *EMA_tf5, *EMA_tf15;
float *HMA, *HMA_2;
float *RSI, *RSI_tf5, *RSI_tf15;
float *SK_tf1, *SK_tf15;
extern float *ADX;

#define PERFORM_REPORT 1
#define PERFORM_MM 0

int main(int argc, char *argv[]){
	int i;
	float best_stop_loss = 0.0005, best_take_profit = 0.0100;	

	float outcome;
	// Trades list
	tp_trade *trades, *trades_IBEX, *trades_SP500;
	int trades_no;
	// Money Management
	float trade_size;
	
    char ticker[12];
    
    strcpy(ticker, "IBEX");

	printf("\n\n***** Analyzing %s *****\n\n", ticker);

	sample = (tp_sample *)malloc(MAX_SAMPLES * sizeof(tp_sample));
	read_data(ticker, sample);
	
	printf("%d samples read\n", SAMPLES);
	
	// TF1'
	SMA = (float *)malloc(SAMPLES * sizeof(float));
	calculate_SMA(sample, SMA_PERIOD, 1, SMA);	

	EMA = (float *)malloc(SAMPLES * sizeof(float));
	calculate_EMA(sample, EMA_PERIOD, 1, EMA);
	
	RSI = (float *)malloc(SAMPLES * sizeof(float));
	calculate_RSI(sample, 14, 1, RSI);

//	ADX = (float *)malloc(SAMPLES * sizeof(float));
//	calculate_ADX(sample, 14);
	
	// TF5'	
	SMA_tf5 = (float *)malloc(SAMPLES * sizeof(float));
	calculate_SMA(sample, SMA_PERIOD, 5, SMA_tf5);	

	EMA_tf5 = (float *)malloc(SAMPLES * sizeof(float));
	calculate_EMA(sample, EMA_PERIOD, 5, EMA_tf5);
	
	RSI_tf5 = (float *)malloc(SAMPLES * sizeof(float));
	calculate_RSI(sample, 9, 5, RSI_tf5);
	
	// TF15'	
	SMA_tf15 = (float *)malloc(SAMPLES * sizeof(float));
	calculate_SMA(sample, SMA_PERIOD, 15, SMA_tf15);	

	EMA_tf15 = (float *)malloc(SAMPLES * sizeof(float));
	calculate_EMA(sample, EMA_PERIOD, 15, EMA_tf15);
	
	RSI_tf15 = (float *)malloc(SAMPLES * sizeof(float));
	calculate_RSI(sample, 8, 15, RSI_tf15);
	
	SK_tf15 = (float *)malloc(SAMPLES * sizeof(float));
	stoch(sample, 14, 15, SK_tf15);



	HMA = (float *)malloc(SAMPLES * sizeof(float));
	calculate_HMA(sample, 0, 9, HMA);

	HMA_2 = (float *)malloc(SAMPLES * sizeof(float));
	calculate_HMA(sample, 0, 20, HMA_2);

	if (argc > 1 && !strcmp(argv[1], "optimize"))
		optimize(ticker, sample, &best_stop_loss, &best_take_profit);

	best_stop_loss = 0.0025;
	best_take_profit = 0.0195;

	printf("Best SL: %.2f\n", 100*best_stop_loss);
	printf("Best TP: %.2f\n", 100*best_take_profit);
	
	trades	  = (tp_trade *)malloc(SAMPLES * sizeof(tp_trade));
	trades_no = simulate(sample, best_stop_loss, best_take_profit, &outcome, trades);

	#if PERFORM_REPORT
		report(ticker, trades, trades_no, sample, best_stop_loss, best_take_profit);
	#endif
	
	// Save trade list
	trades_IBEX	= (tp_trade *)malloc(trades_no * sizeof(tp_trade));
	memcpy(trades_IBEX, trades, trades_no * sizeof(tp_trade));
	
    free(EMA);
	free(RSI);
	free(trades);
    
    #if PERFORM_MM
    	sample_MM = (tp_sample_MM *)malloc(MAX_SAMPLES * sizeof(tp_sample_MM));
		create_samples_MM(sample, sample[SAMPLES-1].day_no + 1, sample_MM);

		if (optimize_money_management(sample_MM, trades_IBEX, trades_SP500, trades_no, &trade_size)){
			printf("Optimal trade size: %.1f\n", trade_size);
			report_money_management(sample_MM, ticker, trades);
			max_drawdown(sample_MM, (sample[SAMPLES-1].day_no + 1)*60*13);
		}
		else
			printf("No trade size satisfied the MM restrictions\n");
	#endif   
	
    return 0;
}


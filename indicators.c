#include "indicators.h"
#include <stdlib.h>

extern int SAMPLES;
float *ADX;
float *DI_periods;
float *DI_periods_n;

// MACD
float *MACD_line;
float *MACD_signal_line;


void calculate_SMA(tp_sample *sample, int periods, int tf, float *destination){
	int i, j;
	float cum;
	
	for (i = periods * tf - 1; i < SAMPLES; i += tf){
		cum = 0;
		for (j = i; j > i - periods * tf; j -= tf)
			cum += sample[j].close;
	
		destination[i] = cum / periods;
		
		// Temp: copy value to the next n intra-TFn samples
		for (j = i + 1; j < i + tf && j < SAMPLES; j++)
			destination[j] = destination[i];
	}
}

void calculate_EMA(tp_sample *sample, int periods, int tf, float *destination){
	int i, j;
	float cum;
	
	// SMA for the first sample
	cum = 0;
	for (i = periods * tf - 1; i > periods * tf - 1 - periods * tf; i -= tf)
		cum += sample[i].close;	
	
	destination[periods * tf - 1] = cum / periods;
	// Temp: copy value to the next n intra-TFn samples
	for (i = periods * tf; i < periods * tf + tf && i < SAMPLES; i++)
		destination[i] = destination[periods * tf - 1];
	
	// EMA for the rest of samples. Smoothing constant is 2 / (periods + 1)
	for (i = periods * tf + tf - 1; i < SAMPLES; i += tf){
		destination[i] = 2 * (sample[i].close - destination[i - tf]) / (periods + 1) + destination[i - tf];
		// Temp: copy value to the next n intra-TFn samples
		for (j = i + 1; j < i + tf && j < SAMPLES; j++)
			destination[j] = destination[i];
	}	
}

float WMA(tp_sample *samplee, int sample_no, int periods){
	// WMA = ( P*n + P(1)*(n-1) + ... + P(n-1)*1) / (n * (n + 1) / 2)
	int i;
	float cum = 0;
	
	for (i = periods; i > 0; i--)
		cum += (float)(i) * samplee[sample_no - periods + i].close;

	return cum / ((float)(periods) * ((float)(periods + 1)) / 2);
}

void calculate_HMA(tp_sample *sample, int sample_no, int periods, float *HMA){
	// HMA = WMA(2 * WMA(n/2) - WMA(n)), sqrt(n))	
	int i, j;
	float cum;
	tp_sample *sample_temp;
	
	sample_temp = (float *)malloc(SAMPLES * sizeof(tp_sample));
	
	// Create a time serie with the WMAs	
	for (i = periods - 1; i < SAMPLES; i++){
		sample_temp[i].close = 2 * WMA(sample, i, periods / 2);
		sample_temp[i].close -= WMA(sample, i, periods);
	}

	for (i = periods - 1 + (int)(sqrt(periods)) - 1; i < SAMPLES; i++)
		HMA[i] = WMA(sample_temp, i, (int)(sqrt(periods)));
}

void calculate_RSI(tp_sample *sample, int periods, int tf, float *destination){
	int i, j;
	float avg_gain = 0, avg_loss = 0;
	float current_gain, current_loss;	
	
	// First sample
	for (i = 2 * tf - 1; i < periods * tf; i += tf){
		if (sample[i].close > sample[i - tf].close)
			avg_gain += sample[i].close - sample[i - tf].close;
		else
			avg_loss += sample[i - tf].close - sample[i].close;
	}
	
	avg_gain /= periods;
	avg_loss /= periods;
	
	destination[periods * tf - 1] = 100 - (100 / (1 + avg_gain / avg_loss));
	
	// Fill intra-TFn samples
	for (i = periods * tf; i < periods * tf + tf; i++){
		//// Copy value
		//destination[i] = destination[periods * tf - 1];
		
		// Take TF1' samples as if they were TFn'
		if (sample[i].close > sample[periods * tf - 1].close){
			current_gain = sample[i].close - sample[periods * tf - 1].close;
			current_loss = 0;
		}			
		else{
			current_gain = 0;
			current_loss = sample[periods * tf - 1].close - sample[i].close;
		}
		
		destination[i] = 100 - (100 / (1 + ((avg_gain * (periods-1) + current_gain) / periods) / ((avg_loss * (periods-1) + current_loss) / periods)));		
	}
		
		
	// Rest of samples
	for (i = periods * tf + tf - 1; i < SAMPLES - tf; i += tf){
		// Now based on the previous value
		if (sample[i].close > sample[i - tf].close){
			current_gain = sample[i].close - sample[i - tf].close;
			current_loss = 0;
		}			
		else{
			current_gain = 0;
			current_loss = sample[i - tf].close - sample[i].close;
		}
		
		avg_gain = (avg_gain * (periods-1) + current_gain) / periods;
		avg_loss = (avg_loss * (periods-1) + current_loss) / periods;

		destination[i] = 100 - (100 / (1 + avg_gain / avg_loss));
		
		// Fill intra-TFn samples
		for (j = i + 1; j < i + 1 + tf; j++){
			//// Copy value
			//destination[j] = destination[i];
			
			// Take TF1' samples as if they were TFn'
			if (sample[j].close > sample[i].close){
				current_gain = sample[j].close - sample[i].close;
				current_loss = 0;
			}			
			else{
				current_gain = 0;
				current_loss = sample[i].close - sample[j].close;
			}
			
			destination[j] = 100 - (100 / (1 + ((avg_gain * (periods-1) + current_gain) / periods) / ((avg_loss * (periods-1) + current_loss) / periods)));
		}
			
	}
}

void stoch(tp_sample *sample, int periods, int tf, float *destination){
	int i, j;
	float min, max;
	
	for (i = periods * tf - 1; i < SAMPLES; i += tf){
		min = sample[i].min;
		max = sample[i].max;
		for (j = i - 1; j > i - periods * tf; j--){
			if (sample[j].min < min)
				min = sample[j].min;
			if (sample[j].max > max)
				max = sample[j].max;
		}

		destination[i] = 100 * (sample[i].close - min) / (max - min);
		
		// Temp: copy value to the next n intra-TFn samples
		for (j = i + 1; j < i + tf && j < SAMPLES; j++)
			destination[j] = destination[i];
	}		

	return ;
}

/*
void calculate_MACD(tp_sample *sample){
	int i;
	float *EMA_a;
	float *EMA_b;
	
	MACD_line = (float *)malloc(SAMPLES * sizeof(float));
	MACD_signal_line = (float *)malloc(SAMPLES * sizeof(float));
	EMA_a = (float *)malloc(SAMPLES * sizeof(float));
	EMA_b = (float *)malloc(SAMPLES * sizeof(float));
	
	// MACD line (EMA12 - EMA26)
	calculate_EMA(sample, 12, EMA_a);
	calculate_EMA(sample, 26, EMA_b);
	for (i = 25; i < SAMPLES; i++)
		MACD_line[i] = EMA_a[i] - EMA_b[i];

//for (i = 26; i < 30; i++)
//	printf("MACD_line: %.2f\n", MACD_line[i]);

//exit(0);
		
	// Signal line (EMA9 of MACDLine)
	
	// Calculate the SMA for the first sample
	float sum = 0;
	for (i = 25; i <= 33; i++)
		sum += MACD_line[i];
	
	MACD_signal_line[33] = sum / 9;
	
	// EMA for the rest of samples. Smoothing constant is 2 / (periods + 1)
	for (i = 34; i < SAMPLES; i++)
		MACD_signal_line[i] = 2 * (MACD_line[i] - EMA[i-1]) / (9 + 1) + EMA[i-1];
	
	free(EMA_a);
	free(EMA_b);
}
*/

void calculate_ADX(tp_sample *sample, int periods){
	int i;
	float *TR;
	float *TR_periods;
	float *DM_1;
	float *DM_1_n;
	float *DM_periods;
	float *DM_periods_n;
	float *DX;
	
	TR = (float *)malloc(SAMPLES * sizeof(float));
	TR_periods = (float *)malloc(SAMPLES * sizeof(float));
	DM_1 = (float *)malloc(SAMPLES * sizeof(float));
	DM_1_n = (float *)malloc(SAMPLES * sizeof(float));
	DM_periods = (float *)malloc(SAMPLES * sizeof(float));
	DM_periods_n = (float *)malloc(SAMPLES * sizeof(float));
	DI_periods = (float *)malloc(SAMPLES * sizeof(float));
	DI_periods_n = (float *)malloc(SAMPLES * sizeof(float));
	DX = (float *)malloc(SAMPLES * sizeof(float));
	ADX = (float *)malloc(SAMPLES * sizeof(float));
	
	// True range
	float high_low_diff, high_close_diff, low_close_diff;
	for (i = 1; i < SAMPLES; i++){
		high_low_diff	  = sample[i].max - sample[i].min;
		high_close_diff = sample[i].max >= sample[i-1].close ? sample[i].max - sample[i-1].close : sample[i-1].close - sample[i].max;
		TR[i] = high_close_diff > high_low_diff ? high_close_diff : high_low_diff;
		low_close_diff  = sample[i].min >= sample[i-1].close ? sample[i].min - sample[i-1].close : sample[i-1].close - sample[i].min;
		TR[i] = low_close_diff > TR[i] ? low_close_diff : TR[i];	
	}

	// Directional Movements
	for (i = 1; i < SAMPLES; i++){
		DM_1[i]   = (sample[i].max - sample[i-1].max > 0) && (sample[i].max - sample[i-1].max > sample[i-1].min - sample[i].min) ? sample[i].max - sample[i-1].max : 0;
		DM_1_n[i] = (sample[i-1].min - sample[i].min > 0) && (sample[i-1].min - sample[i].min > sample[i].max - sample[i-1].max) ? sample[i-1].min - sample[i].min : 0;
	}

	// True range-period
	for (i = 1; i <= periods; i++){
		TR_periods[periods] += TR[i];
	}

	for (i = periods+1; i < SAMPLES; i++)
		TR_periods[i] = TR_periods[i-1] - (TR_periods[i-1] / periods) + TR[i]; 

	// Directional Movements - periods
	for (i = 1; i <= periods; i++){
		DM_periods[periods]   += DM_1[i];
		DM_periods_n[periods] += DM_1_n[i];
	}
	for (i = periods+1; i < SAMPLES; i++){
		DM_periods[i]   = DM_periods[i-1]   - (DM_periods[i-1]   / periods) + DM_1[i];
		DM_periods_n[i] = DM_periods_n[i-1] - (DM_periods_n[i-1] / periods) + DM_1_n[i];
	}

	// Directional Indicator
	for (i = periods; i < SAMPLES; i++){
		DI_periods[i]   = 100 * DM_periods[i] / TR_periods[i];
		DI_periods_n[i] = 100 * DM_periods_n[i] / TR_periods[i];
	}

	// DX
	for (i = periods; i < SAMPLES; i++){
		float DI_periods_diff = DI_periods[i] >= DI_periods_n[i] ?  DI_periods[i] - DI_periods_n[i] : DI_periods_n[i] - DI_periods[i];
		float DI_periods_sum = DI_periods[i] + DI_periods_n[i];
		DX[i]  = 100 * DI_periods_diff / DI_periods_sum;
	}

	// ADX
	for (i = periods; i <= periods*2-1; i++){
		ADX[periods*2-1] += DX[i];
	}
	ADX[periods*2-1] /= periods;
	for (i = periods*2; i < SAMPLES; i++){
		ADX[i]   = (ADX[i-1] * (periods-1) + DX[i]) / periods;
	}

	free(TR);
	free(TR_periods);
	free(DM_1);
	free(DM_1_n);
	free(DM_periods);
	free(DM_periods_n);
//	free(DI_periods);
//	free(DI_periods_n);
	free(DX);	
}
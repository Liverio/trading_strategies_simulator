#include "signals.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

extern tp_sample *sample;
extern int SAMPLES;

extern float *SMA, *SMA_tf5, *SMA_tf15;
extern float *EMA, *EMA_tf5, *EMA_tf15;
extern float *HMA, *HMA_2;
extern float *RSI, *RSI_tf5, *RSI_tf15;
extern float *SK_tf1, *SK_tf15;
extern float *ADX;

#define CONSIDER_LONG  1
#define CONSIDER_SHORT 0
#define CONSIDER_TIME_SLOTS 0

char time_slot(int num_sample){
	int i;
	int sample_time;
	
	#define NUM_SLOTS 1
	int slot[NUM_SLOTS][2] = {{ 900, 1100}};
//	int slot[NUM_SLOTS][2] = {{ 920, 1400},		// IBEX						  	  
//							  {1600, 1700}};
	/*int slot[NUM_SLOTS][3] = {{ 920, 1300},		// IBEX
						  	  {1400, 1430},
						  	  {1450, 1700}};*/
	
	if (!CONSIDER_TIME_SLOTS)
		return 1;
		
	sample_time = atoi(sample[num_sample].time);
	for (i = 0; i < NUM_SLOTS; i++){
		if (sample_time >= slot[i][0] && sample_time < slot[i][1])
			return 1;
	}
	return 0;
}


char find_signal(int num_sample, int *sample_found, tp_direction *direction){
	int i;

	for (i = num_sample; i < SAMPLES-1; i++){
		#if CONSIDER_LONG
			//if ((HMA[i] < HMA_2[i]) && (HMA[i+1] >= HMA_2[i+1]) && (time_slot(i+2))){
			if ((EMA[i] < SMA[i]) && (EMA[i+1] >= SMA[i+1]) && (time_slot(i+2))){
//if ((EMA_tf15[i+1] < SMA_tf15[i+1]) && (EMA[i] < SMA[i]) && (EMA[i+1] >= SMA[i+1]) && (RSI[i+1] > 50) && (time_slot(i+2))){			
//if ((EMA_tf15[i+1] > SMA_tf15[i+1]) && (RSI_tf15[i+1] > 45) && (EMA[i] < SMA[i]) && (EMA[i+1] >= SMA[i+1]) && (time_slot(i+2))){
			//if ((RSI_tf15[i+1] >= 50) && (EMA_tf15[i+1] < SMA_tf15[i+1]) && (EMA[i] < SMA[i]) && (EMA[i+1] >= SMA[i+1]) && (RSI[i+1] >= 0) && (time_slot(i+2))){	// TEMP THIS
//if (SK_tf15[i+1] < 50 && EMA_tf15[i+1] < SMA_tf15[i+1] && RSI_tf15[i+1] >= 50 && EMA[i] < SMA[i] && EMA[i+1] >= SMA[i+1] && RSI[i+1] >= 0 && time_slot(i+2)){
				*sample_found = i + 1;
				*direction = LONG;
				return 1;
			}
		#endif
	
		#if CONSIDER_SHORT
			if ((EMA[i] > SMA[i]) && (EMA[i+1] <= SMA[i+1]) && (RSI[i+1] < 50) && (time_slot(i+2))){
//if ((EMA_tf15[i+1] < SMA_tf15[i+1]) && (RSI_tf15[i+1] < 45) && (EMA[i] > SMA[i]) && (EMA[i+1] <= SMA[i+1]) && (time_slot(i+2))){				
//if ((RSI_tf15[i+1] <= 45) && (EMA[i] > SMA[i]) && (EMA[i+1] <= SMA[i+1]) && (RSI[i+1] <= 100) && (time_slot(i+2))){
			//if ((RSI_tf15[i+1] <= 45) && (EMA_tf15[i+1] > SMA_tf15[i+1]) && (EMA[i] > SMA[i]) && (EMA[i+1] <= SMA[i+1]) && (RSI[i+1] <= 100) && (time_slot(i+2))){	// TEMP THIS	//if ((RSI_tf15[i+1] <= 40) && (EMA_tf15[i+1] > SMA_tf15[i+1]) && (EMA[i] > SMA[i]) && (EMA[i+1] <= SMA[i+1]) && (RSI[i+1] <= 100) && (time_slot(i+2))){	// TEMP THIS
//if (SK_tf15[i+1] > 50 && (EMA_tf15[i+1] > SMA_tf15[i+1]) && (RSI_tf15[i+1] <= 50) && (EMA[i] > SMA[i]) && (EMA[i+1] <= SMA[i+1]) && (RSI[i+1] <= 100) && (time_slot(i+2))){
				*sample_found = i + 1;
				*direction = SHORT;
				return 1;
			}
		#endif
	}
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include "money_management.h"
#include <math.h>

extern float *margin, *value, *nominal_long, *nominal_short;

void max_drawdown(tp_sample_MM *sample, int samples){
	float max_drawdown = 0;
	int i = 0, j, k;
	int peak_sample, valley_sample;
	
	printf("\nMax drawdowns:\n");
	while (i < samples-3){
		// Begin of a decline not due to a cashout (neither regular cashout nor end-of-year)
		if ((value[i] > value[i+1]) && (sample[i].day_no == sample[i+1].day_no)){
			peak_sample = i;
			valley_sample = i+1;
			j = i+2;
			while ((j < samples-1) && !((sample[j].day_no != sample[j-1].day_no) && (value[j] != value[j-1])) && (value[j] < value[peak_sample])){
				if (value[j] < value[valley_sample]){
					valley_sample = j;
				}
				j++;
			}
			
			// Switching year: show Max DD
			if (sample[j].year_no != sample[j-1].year_no){
				printf("Max DD %d:\t%.1f%%\n", sample[j-1].year_no, 100 * max_drawdown);
				max_drawdown = 0;
			}
	
			// Biggest drawdown
			if ((value[peak_sample] - value[valley_sample]) / value[peak_sample] > max_drawdown){
				max_drawdown = (value[peak_sample] - value[valley_sample]) / value[peak_sample];
				//printf("Max DD: %.1f%% from %s %s to %s %s\n", 100 * max_drawdown, sample[i].date, sample[i].time, sample[valley_sample].date, sample[valley_sample].time);
			}			
			i = j;
		}
		else{			
			i++;
			if (sample[i].year_no != sample[i-1].year_no){
				printf("Max DD %d:\t%.1f%%\n", sample[i-1].year_no, 100 * max_drawdown);
				max_drawdown = 0;
			}
		}
	}
}

/*
char time_slot_last_trade(int initial_sample){
	int i;
	int current_session;
		
	current_session = sample[initial_sample].day_no;
	i = initial_sample;
	
	while (i < SAMPLES-1 && sample[i].day_no == current_session){
		if (longs[i] == 0 && shorts[i] == 0)
			return sample[i].time[1] - '2';
		i++;
	}
	return '7' - '2';
}

void end_of_session_stats(){
	int i;
	int current_session;
	
	// Init stats
	for (i = '2' - '2'; i <= '7' - '2'; i++)
		end_of_session[i] = 0;
	
	i = 0;
	while (i < SAMPLES-1){
		// New trades only from 9 to 12. Check when last trade is closed each session
		if (sample[i].time[1] == '2'){
			current_session = sample[i].day_no;
//printf("\nLooking EOS for day %d...", current_session);
			end_of_session[time_slot_last_trade(i)]++;
//printf("\tIt was %d...\n", time_slot_last_trade(i));
//if (current_session == 10) exit(0);
			// Move to next session
			while (i < SAMPLES-1 && sample[i].day_no == current_session)
				i++;			
		}
		else{
			i++;
		}		
	}	
	
	printf("\nSession finished:\n");
	for (i = '2' - '2'; i <= '7' - '2'; i++)
		printf("\t%d - %d: %.1f%%\n", i + 12, i + 12 + 1, 100 * (float)(end_of_session[i]) / (float)(sample[SAMPLES-1].day_no));
//printf("%d\n", end_of_session[i]);

//printf("Added is %d\n", end_of_session[0] + end_of_session[1] + end_of_session[2] + end_of_session[3] + end_of_session[4] + end_of_session[5]);
//printf("days_no is: %d %d", sample[SAMPLES-1].day_no, sample[SAMPLES-2].day_no);

	printf("\n");
	printf("\t< %d: %.1f%%\n", 12 + 1, 100 * (float)(end_of_session[0]) / (float)(sample[SAMPLES-1].day_no));
	printf("\t< %d: %.1f%%\n", 12 + 2, 100 * (float)(end_of_session[0] + end_of_session[1]) / (float)(sample[SAMPLES-1].day_no));
	printf("\t< %d: %.1f%%\n", 12 + 3, 100 * (float)(end_of_session[0] + end_of_session[1] + end_of_session[2]) / (float)(sample[SAMPLES-1].day_no));
	printf("\t< %d: %.1f%%\n", 12 + 4, 100 * (float)(end_of_session[0] + end_of_session[1] + end_of_session[2] + end_of_session[3]) / (float)(sample[SAMPLES-1].day_no));
	printf("\t< %d: %.1f%%\n", 12 + 5, 100 * (float)(end_of_session[0] + end_of_session[1] + end_of_session[2] + end_of_session[3] + end_of_session[4]) / (float)(sample[SAMPLES-1].day_no));
	printf("\t< %d: %.1f%%\n", 12 + 6, 100 * (float)(end_of_session[0] + end_of_session[1] + end_of_session[2] + end_of_session[3] + end_of_session[4] + end_of_session[5]) / (float)(sample[SAMPLES-1].day_no));

}
*/

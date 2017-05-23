#include "report.h"
#include "indicators.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"

extern int SAMPLES;
extern float *SMA, *SMA_tf15;
extern float *EMA, *EMA_tf15;
extern float *RSI, *RSI_tf15;

#define SHOW_INDICATORS 1
#define CONSIDER_TIME_SLOTS 1

void report(char *ticker, tp_trade *trades, int trades_no, tp_sample *sample, float stop_loss, float take_profit){
	int i, j;
	FILE *f_report, *f_dayly_outcome, *f_monthly_outcome;
	int trade_no;
	int sample_in, sample_out;
	tp_direction direction;
	float outcome;
	int won = 0, loss = 0;
	char path_report[100], path_monthly_outcome[100], path_dayly_outcome[100];
	float *dayly_outcome_curve, *monthly_outcome_curve;
	int days_no = sample[SAMPLES-1].day_no + 1, months_no = sample[SAMPLES-1].month_no + 1;
	int best_trade_no;
	float best_trade = 0;
	// Time slots
	int won_time[9]  = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	int loss_time[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	float outcome_time[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	int slot;

	sprintf(path_report, "./output_data/%s/report.txt", ticker);
	sprintf(path_dayly_outcome, "./output_data/%s/dayly_outcome.txt", ticker);
	sprintf(path_monthly_outcome, "./output_data/%s/monthly_outcome.txt", ticker);
	
	f_report = fopen(path_report, "w");
	f_dayly_outcome = fopen(path_dayly_outcome, "w");
	f_monthly_outcome = fopen(path_monthly_outcome, "w");
	
	dayly_outcome_curve = (float *)malloc(days_no * sizeof(float));
	monthly_outcome_curve = (float *)malloc(months_no * sizeof(float));
	
	// Reset outcomes
	for (i = 0; i < days_no; i++)
		dayly_outcome_curve[i] = 0;
	for (i = 0; i < months_no; i++)	
		monthly_outcome_curve[i] = 0;

	// Slots TF5'
	int set_i, set_j;
	float outcome_TF5[9][12];
	int won_time_TF5[9][12];
	int loss_time_TF5[9][12];
	
	for (i = 0; i < 9; i++)
		for (j = 0; j < 12; j++){
			outcome_TF5[i][j] = 0;
			won_time_TF5[i][j] = 0;
			loss_time_TF5[i][j] = 0;
		}
		

	for (trade_no = 0; trade_no < trades_no; trade_no++){
		direction  = trades[trade_no].direction;
		sample_in  = trades[trade_no].sample_in;
		sample_out = trades[trade_no].sample_out;
		outcome    = trades[trade_no].outcome;
		
		if (outcome > 0)
			won++;
		else
			loss++;
			
		if (outcome > best_trade){
			best_trade = outcome;
			best_trade_no = trade_no;
		}		

		fprintf(f_report, "%s\n", direction == LONG ? "LONG" : "SHORT");
		fprintf(f_report, "IN:  %s\t%s\t@%.1f\n", sample[sample_in].date, sample[sample_in].time, sample[sample_in].open);	//fprintf(f_report, "IN:  %s\t%s\n", sample[sample_in].date, sample[sample_in].time);
		fprintf(f_report, "OUT: %s\t%s\t@%.1f\t%2.4f%%\n", sample[sample_out].date, sample[sample_out].time, sample[sample_out].open, 100 * outcome);
		#if SHOW_INDICATORS
			fprintf(f_report, "\tSMA %.1f\tEMA %.1f\n", SMA[sample_in - 2], EMA[sample_in - 2]);
			fprintf(f_report, "\tSMA %.1f\tEMA %.1f\tRSI_TF1 %.1f\n", SMA[sample_in - 1], EMA[sample_in - 1], RSI[sample_in - 1]);
			fprintf(f_report, "\tSMA_TF15 %.1f\tEMA_TF15 %.1f\tRSI_TF15 %.1f\n", SMA_tf15[sample_in - 1], EMA_tf15[sample_in - 1], RSI_tf15[sample_in - 1]);
		#endif

		// Outcome evolution
		for (i = sample[sample_out].day_no; i < days_no; i++)
			dayly_outcome_curve[i] += 100 * outcome;
		for (i = sample[sample_out].month_no; i < months_no; i++)
			monthly_outcome_curve[i] += 100 * outcome;
	
		#if CONSIDER_TIME_SLOTS
			// TF1h
			slot = sample[sample_in].time_slot;		
			outcome_time[slot] += outcome;
	
			if (outcome > 0)
				(won_time[slot])++;
			else
				(loss_time[slot])++;
			
			// TF5m
			for (j = 0; j < 9; j++){
				for (i = 0; i < 12; i++){
					if (i > 0 && atoi(sample[sample_in].time) <= 900 + i * 5 + j * 100 && atoi(sample[sample_in].time) >= 900 + i * 5 + j * 100 - 4){
						set_i = i;
						set_j = j;
					}
					else if (i == 0 && atoi(sample[sample_in].time) <= 900 + i * 5 + j * 100 && atoi(sample[sample_in].time) >= 900 + i * 5 + j * 100 - 44){
						set_i = i;
						set_j = j;
					}
				}
			}
			//printf("sample time: %s was assigned to set %d, %d -> %d\n", sample[sample_in].time, set_j, set_i, 900 + set_i * 5 + set_j * 100);
						
			outcome_TF5[set_j][set_i] += outcome;	
		
			if (outcome > 0)
				(won_time_TF5[set_j][set_i])++;
			else
				(loss_time_TF5[set_j][set_i])++;
		#endif
	}

	#if CONSIDER_TIME_SLOTS
		// TF1h
		for (slot = 0; slot < 9; slot++){
			printf("%d - %d\n", 9+slot, 9+slot+1);
			if (won_time[slot] + loss_time[slot] == 0){
				printf("\tNo trades\n");
			}
			else{
				printf("\tTrades: %d\n", won_time[slot] + loss_time[slot]);
				printf("\tWon: %d\t%.1f%%\n", won_time[slot], 100 * (float)(won_time[slot]) / ((float)(won_time[slot]) + (float)(loss_time[slot])));
				printf("\tEV : %.3f (%% / trade)\n", 100 * outcome_time[slot] / ((float)(won_time[slot]) + (float)(loss_time[slot])));
			}
		}
		
		// TF5m
/*		for (j = 0; j < 9; j++){
			for (i = 0; i < 12; i++){
				printf("%d\t", 900 + i * 5 + j * 100);	//printf("%d\n", 900 + i * 5 + j * 100);
				if (won_time_TF5[j][i] + loss_time_TF5[j][i] == 0){
					printf("\tNo trades\n");
				}
				else{
					//printf("\tTrades: %d\n", won_time_TF5[j][i] + loss_time_TF5[j][i]);
					//printf("\tWon: %.1f%%\n", 100*(float)(won_time_TF5[j][i]) / ((float)(won_time_TF5[j][i]) + (float)(loss_time_TF5[j][i])));
					//printf("\tEV : %.3f (%% / trade)\n", 100 * outcome_TF5[j][i] / ((float)(won_time_TF5[j][i]) + (float)(loss_time_TF5[j][i])));
					printf("\t%d\t%.3f\n", won_time_TF5[j][i] + loss_time_TF5[j][i], 100 * outcome_TF5[j][i] / ((float)(won_time_TF5[j][i]) + (float)(loss_time_TF5[j][i])));
				}
			}
		}*/
	#endif
	
	printf("Outcome\t: %.1f%%\n", dayly_outcome_curve[days_no-1]);
	
	// Year-by-year outcome	
	float previous_outcome = 0;
	char current_year[2];
	current_year[1] = sample[0].date[3];
	current_year[0] = sample[0].date[2];
	
	for (i = 0; i < days_no; i++){
		// Locate sample corresponding to current day_no
		while (sample[j].day_no != i)
			j++;
		// New year
		if ((sample[j].date[2] != current_year[0]) || (sample[j].date[3] != current_year[1])){
			printf("\t%c%c%c%c: %.1f%%\n", '2', '0', current_year[0], current_year[1], dayly_outcome_curve[i-1] - previous_outcome);
			current_year[1] = sample[j].date[3];
			current_year[0] = sample[j].date[2];
			previous_outcome = dayly_outcome_curve[i-1];
		}	
	}
	printf("\t%c%c%c%c: %.1f%%\n", '2', '0', current_year[0], current_year[1], dayly_outcome_curve[i-1] - previous_outcome);
	
	printf("Trades\t: %d\n", trades_no);
	printf("\t- Won : %5d (%.1f%%)\n", won,  100 * (float)(won) / ((float)(won + loss)));
	printf("\t- Loss: %5d (%.1f%%)\n", loss, 100 * (float)(loss) / ((float)(won + loss)));
	printf("\t- EV  : %.3f (%% / trade)\n\n", dayly_outcome_curve[days_no-1] / trades_no);
	printf("Best trade:\t%s\n", trades[best_trade_no].direction == LONG ? "LONG" : "SHORT");
	printf("\t\tIN:\t%s %s\n", sample[trades[best_trade_no].sample_in].date, sample[trades[best_trade_no].sample_in].time);
	printf("\t\tOUT:\t%s %s\t%.1f%%\n", sample[trades[best_trade_no].sample_out].date, sample[trades[best_trade_no].sample_out].time, 100 * trades[best_trade_no].outcome);
	
	// Dump outcome curves
	for (i = 0; i < days_no; i++)
		fprintf(f_dayly_outcome, "%.2f\n", dayly_outcome_curve[i]);
	for (i = 0; i < months_no; i++)
		fprintf(f_monthly_outcome, "%.2f\n", monthly_outcome_curve[i]);
			
	fclose(f_report);
	fclose(f_dayly_outcome);
	fclose(f_monthly_outcome);
}

#include <stdio.h>
#include <stdlib.h>
#include "money_management.h"
#include "mm_stats.h"
#include <math.h>

extern int SAMPLES;
extern tp_sample *sample;
float *margin, *value, *nominal_long, *nominal_short;
int *longs, *shorts;
unsigned long *futures_long, *futures_short;
// Cashouts
float yearly_profit[17];

// Search the #sample for given date and time
int search_sample_MM(int initial_sample, tp_sample_MM *sample_MM, char *date, char *time){
	int sample_no = initial_sample;
	
	while (strcmp(date, sample_MM[sample_no].date) || strcmp(time, sample_MM[sample_no].time))
		sample_no++;		

	return sample_no;
}

void process_SP500_trades(tp_sample_MM *sample_MM, char *date){
	int sample_no = 0;
	
	while (strcmp(sample_MM[sample_no].date, date) && longs[sample_no] != 0 && shorts[sample_no] != 0)
		sample_no++;
}


char check_trade_size(tp_sample_MM *sample_MM, tp_trade *trades_IBEX, tp_trade *trades_SP500, int trades_no, float trade_size, int *actual_trades){
	int i;
	int trade_no;
	int longs_shorts_diff;
	unsigned long futures_no;
	float future_value;
	float nominal;
	int sample_in;
	int sample_out;
	int previous_trade_sample_in;
	int sample_no;

	FILE *f_dayly_outcome;
	char path_dayly_outcome[100];
	float *dayly_outcome_curve;
	int days_no = sample[SAMPLES-1].day_no + 1;
	int years_no = sample[SAMPLES-1].year_no + 1;
	int samples_MM_no = days_no * 60 * 13;
	
	for (i = 0; i < years_no; i++)
		yearly_profit[i] = 0;
	
	sprintf(path_dayly_outcome, "./output_data/IBEX/dayly_outcome_after_MM.txt");
	f_dayly_outcome = fopen(path_dayly_outcome, "w");
	dayly_outcome_curve = (float *)malloc(days_no * sizeof(float));
	for (i = 0; i < days_no; i++)
		dayly_outcome_curve[i] = 0;

	*actual_trades = 0;
	
	// Init margin, value and simulateneous longs/shorts
	for (i = 0; i < samples_MM_no; i++){
		margin[i] = INITIAL_CAPITAL;
		value[i] = INITIAL_CAPITAL;
		longs[i]  = 0;
		shorts[i] = 0;
		nominal_long[i] = 0;
		nominal_short[i] = 0;
		futures_long[i] = 0;
		futures_short[i] = 0;
	}
	
	printf("trades_no: %d\n", trades_no);

	for (trade_no = 0; trade_no < trades_no; trade_no++){
		// Look for the corresponding #sample in and out, in samples_MM
		previous_trade_sample_in = trade_no > 0 ? sample_in : 0;
		sample_in = search_sample_MM(previous_trade_sample_in, sample_MM, trades_IBEX[trade_no].date_in, trades_IBEX[trade_no].time_in);
		sample_out = search_sample_MM(sample_in, sample_MM, trades_IBEX[trade_no].date_out, trades_IBEX[trade_no].time_out);		

		// **** Check if there was room for SP500 trading ****
//		if (trade_no > 0 && strcmp(sample_MM[sample_in].date, sample_MM[previous_trade_sample_in].date))
//			process_SP500_trades(sample_MM, sample_MM[previous_trade_sample_in].date);

		// Cash out profit when CASHOUT_THRESHOLD is reached at the end of the session OR Cashout/Deposit when a new year begins
		if ((value[sample_in] >= CASHOUT_THRESHOLD && strcmp(sample_MM[sample_in].date, sample_MM[previous_trade_sample_in].date)) || (sample_MM[sample_in].year_no != sample_MM[previous_trade_sample_in].year_no)){
			yearly_profit[sample_MM[previous_trade_sample_in].year_no] += value[sample_in] - INITIAL_CAPITAL;

			// Make cashout effective since 9.00
			i = sample_in - 1;
			while (!strcmp(sample_MM[i].date, sample_MM[sample_in].date)){
				value[i] = INITIAL_CAPITAL;
				i--;
			}
			for (i = sample_in; i < samples_MM_no; i++)
				value[i] = INITIAL_CAPITAL;
		}

		// Number of future contracts that fits trade_size
		future_value = 10 * trades_IBEX[trade_no].price_in;
		futures_no = trade_size * value[sample_in] / future_value;
		nominal = futures_no * future_value;

		// Do not allow simultaneous longs and shorts (Visual Chart restriction)
		if ((DO_NOT_OVERLAP_LONG_SHORT && (trades_IBEX[trade_no].direction == LONG && shorts[sample_in] == 0) || (trades_IBEX[trade_no].direction == SHORT && longs[sample_in] == 0)) || !DO_NOT_OVERLAP_LONG_SHORT){
			// Do not allow the system the possibility of being leveraged higher than MAX_LEVERAGE
			if (((trades_IBEX[trade_no].direction == LONG)  && (nominal_long[sample_in] + nominal)  <= MAX_LEVERAGE * value[sample_in]) |
				((trades_IBEX[trade_no].direction == SHORT) && (nominal_short[sample_in] + nominal) <= MAX_LEVERAGE * value[sample_in])){
				(*actual_trades)++;
				
				// Outcome evolution
				for (i = sample_MM[sample_out].day_no; i < days_no; i++)
					dayly_outcome_curve[i] += 100 * trades_IBEX[trade_no].outcome;
				
				// Update account value
				for (i = sample_out; i < samples_MM_no; i++){
					value[i] += nominal * trades_IBEX[trade_no].outcome;
					// Busted
					if (value[i] < 0)
						return 0;	
				}
			
				// Update long/short opened
				if (trades_IBEX[trade_no].direction == LONG){
					for (i = sample_in; i < sample_out; i++){
						nominal_long[i] += nominal;
						futures_long[i] += futures_no;
						longs[i]++;
					}
				}
				else{
					for (i = sample_in; i < sample_out; i++){
						nominal_short[i] += nominal;
						futures_short[i] += futures_no;
						shorts[i]++;
					}
				}
			}
		}
		
		// Last year float profit/loss
		if (trade_no == trades_no - 1)
			yearly_profit[sample_MM[sample_in].year_no] += value[sample_in] - INITIAL_CAPITAL;
	}
	
	// Update margin
	for (i = 0; i < SAMPLES; i++){
		longs_shorts_diff = longs[i] > shorts[i] ? longs[i] - shorts[i] : shorts[i] - longs[i];
		margin[i] = value[i] - longs_shorts_diff * (trade_size * WARRANTY) * value[i];
		// Falls under margin THRESHOLD: discard trade size
		if (margin[i] < MARGIN_THRESHOLD * value[i]) 
			return 0;
	}

	// Dump outcome curve
	for (i = 0; i < days_no; i++)
		fprintf(f_dayly_outcome, "%.2f\n", dayly_outcome_curve[i]);
	fclose(f_dayly_outcome);

	return 1;	
}

char optimize_money_management(tp_sample_MM *sample_MM, tp_trade *trades_IBEX, tp_trade *trades_SP500, int trades_no, float *trade_size){
	int samples_MM_no = (sample[SAMPLES - 1].day_no + 1) * 60 *13;
	margin = (float *)malloc(samples_MM_no * sizeof(float));
	value = (float *)malloc(samples_MM_no * sizeof(float));
	longs = (int *)malloc(samples_MM_no * sizeof(int));
	futures_long = (unsigned long *)malloc(samples_MM_no * sizeof(unsigned long));
	nominal_long = (float *)malloc(samples_MM_no * sizeof(float));
	futures_short = (unsigned long *)malloc(samples_MM_no * sizeof(unsigned long));
	shorts = (int *)malloc(samples_MM_no * sizeof(int));
	nominal_short = (float *)malloc(samples_MM_no * sizeof(float));
	int actual_trades = 0;

	// Explore trade sizes until margin does not fall under THRESHOLD
	for (*trade_size = MAX_LEVERAGE; *trade_size > 0; *trade_size -= 0.1){
		printf("Checking trade size %.1f... ", *trade_size);
		if (check_trade_size(sample_MM, trades_IBEX, trades_SP500, trades_no, *trade_size, &actual_trades)){
			printf("OK\n");
			printf("Actual trades: %d\n", actual_trades);
			return 1;
		}
		printf("FAILED\n");
	}
	
	return 0;
}

void report_money_management(tp_sample_MM *sample_MM, char *ticker, tp_trade *trades){
	int i;
	float year_initial_capital = INITIAL_CAPITAL, year_profit = 0;
	FILE *f_report;
	char path[100];
	int years_no = sample[SAMPLES-1].year_no + 1;

	sprintf(path, "./output_data/%s/MM_report_0.txt", ticker);
	f_report = fopen(path, "w");
	
	int num_samples = (sample[SAMPLES - 1].day_no + 1) * 60 * 13;

	#define	SAMPLE_BY_SAMPLE 1

	for (i = 0; i < num_samples - 1; i++){
		#if SAMPLE_BY_SAMPLE
			// New year in a new file
			if (i > 0 && sample_MM[i].year_no != sample_MM[i-1].year_no){
				fclose(f_report);
				sprintf(path, "./output_data/%s/MM_report_%d.txt", ticker, sample_MM[i].year_no);
				f_report = fopen(path, "w");
			}

			fprintf(f_report, "%s\t%s:\tMargin\t%9.1f\tValue\t%9.1f\tLongs\t%3d\t(%4d)\tShorts\t%3d\t(%4d)\tLeverage %.1f\n", sample_MM[i].date, sample_MM[i].time, margin[i], value[i], longs[i], futures_long[i], shorts[i], futures_short[i], fabs((nominal_long[i] - nominal_short[i]) / value[i]));			
		#endif
		#if !SAMPLE_BY_SAMPLE
			if ((sample_MM[i].day_no != sample_MM[i+1].day_no) || (i == num_samples - 2))
				fprintf(f_report, "%s\t%s:\tMargin\t%9.1f\tValue\t%9.1f\tLongs\t%3d (%4d)\tShorts\t%3d (%4d)\tLeverage %.1f\n", sample_MM[i].date, sample_MM[i].time, margin[i], value[i-1], longs[i-1], futures_long[i-1], shorts[i-1], futures_short[i-1], fabs((nominal_long[i-1] - nominal_short[i-1]) / value[i-1]));
		#endif
	}
	fclose(f_report);
	
	for (i = 0; i < years_no; i++)
		printf("Year %2d:\t%5.1f%%\t(%10.1f euros)\n", i, 100 * yearly_profit[i] / INITIAL_CAPITAL, yearly_profit[i]);
	
//	max_drawdown();
//	end_of_session_stats();
	return ;
}


void create_samples_MM(tp_sample *sample, int days_no, tp_sample_MM *sample_MM){
	int i, j, k;
	char current_time[5];
	char current_date[9];

	j = 0;
	for (i = 0; i < days_no; i++){
		while (sample[j].day_no != i)
			j++;
		
		strcpy(current_date, sample[j].date);
		
		// Create samples from 9.00 to 22.00
		strcpy(current_time, "0900");
		for (k = 0; k < 60*13; k++){
			strcpy(sample_MM[(i * 60*13) + k].time, current_time);
			strcpy(sample_MM[(i * 60*13) + k].date, current_date);
			sample_MM[(i * 60*13) + k].day_no = i;
			sample_MM[(i * 60*13) + k].month_no = sample[j].month_no;
			sample_MM[(i * 60*13) + k].year_no = sample[j].year_no;

			// Get next time stamp
			if (current_time[3] < '9'){
				current_time[3]++;
			}
			else if (current_time[2] < '5'){
				current_time[2]++;
				current_time[3] = '0';
			}
			else if (current_time[1] < '9'){
				current_time[1]++;
				current_time[2] = '0';
				current_time[3] = '0';
			}
			else{
				current_time[0]++;
				current_time[1] = '0';
				current_time[2] = '0';
				current_time[3] = '0';
			}
		}
	}
	printf("total samples created: %d (max is %d)\n", days_no * 60 * 13, MAX_SAMPLES);
}




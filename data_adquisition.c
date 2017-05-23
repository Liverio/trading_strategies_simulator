#include <stdio.h>
#include <string.h>
#include "data_adquisition.h"

char report_data;
int SAMPLES;

void read_data(char *ticker, tp_sample *sample){
	// Input data
	FILE *f_data;
	char path[100];
	char trash[100];

	// Sample fields
	char date[9];
	char time[7];
	float open, high, low, close;
	int day_no, month_no, year_no;
	int time_slot;

	sprintf(path, "../../input_data/%s/1m.txt", ticker);
	f_data = fopen(path, "r");

	// Read and store end-of-day data
	// Format:	<TICKER>,<PER>,<DTYYYYMMDD>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>,<OPENINT>
	//			.IBEX,I,20050103,090100,9085.2,9085.2,9085.2,9085.2,2710,0
	// Store OPEN, HIGH, LOW, CLOSE, consider DATE to know when to go ahead, and discard anything else

	// Skip header
	fscanf(f_data, "%s", trash);
	
	SAMPLES = 0;
	
	int close_time;	
	if (!strcmp(ticker, "DOW") || !strcmp(ticker, "SP500") || !strcmp(ticker, "NDX"))
		close_time = 2200;	// close_time = 2300;	
	else
		close_time = 2100;	//close_time = 1735;	
		
	// Skip data until initial date is reached
	char year[5]; 
	do{
		fscanf(f_data, "%[^,],%[^,],%[^,],%[^,],%f,%f,%f,%f,%s", trash, trash, date, time, &open, &high, &low, &close, trash);
		date[8] = '\0';
		memcpy(year, date, 4*sizeof(char));
		year[4] = '\0';
	} while (strcmp(year, initial_date));
	
	do{
		// Discard data where time is later than close_time
		time[4] = '\0';
//		if ((!strcmp(ticker, "SP500") && atoi(time) >= 1400 && atoi(time) <= close_time) || ((!strcmp(ticker, "IBEX") || !strcmp(ticker, "FIBEX") || !strcmp(ticker, "SX5E") || !strcmp(ticker, "FSX5E")) && atoi(time) <= close_time)){
//		if (atoi(time) <= close_time){
			memcpy(sample[SAMPLES].date, date, 8*sizeof(char));
			memcpy(sample[SAMPLES].time, time, 4*sizeof(char));
			sample[SAMPLES].date[8] = '\0';
			sample[SAMPLES].time[4] = '\0';
			sample[SAMPLES].open  = open;
			sample[SAMPLES].close = close;
			sample[SAMPLES].max = high;
			sample[SAMPLES].min = low;
			// #day will be used to optimize according to std dev
			if (SAMPLES == 0){
				day_no = 0;
				month_no = 0;
				year_no = 0;
				time_slot = 0;
			}
			else if (strcmp(sample[SAMPLES].date, sample[SAMPLES-1].date)){
				day_no++;
				// Next month
				if ((sample[SAMPLES].date[4] != sample[SAMPLES-1].date[4]) || (sample[SAMPLES].date[5] != sample[SAMPLES-1].date[5]))
					month_no++;
				// Next year
				if ((sample[SAMPLES].date[2] != sample[SAMPLES-1].date[2]) || (sample[SAMPLES].date[3] != sample[SAMPLES-1].date[3]))
					year_no++;
				time_slot = 0;
			}
			else if ((sample[SAMPLES].time[0] != sample[SAMPLES-1].time[0]) || (sample[SAMPLES].time[1] != sample[SAMPLES-1].time[1]))
				time_slot++;
			
			sample[SAMPLES].day_no = day_no;
			sample[SAMPLES].month_no = month_no;
			sample[SAMPLES].year_no = year_no;
			sample[SAMPLES].time_slot = time_slot;
			
			SAMPLES++;
//		}
	} while (fscanf(f_data, "%[^,],%[^,],%[^,],%[^,],%f,%f,%f,%f,%s", trash, trash, date, time, &open, &high, &low, &close, trash) != -1);
		
	fclose(f_data);
	return ;
}

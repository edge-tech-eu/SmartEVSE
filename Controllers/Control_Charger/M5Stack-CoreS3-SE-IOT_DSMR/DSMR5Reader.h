#ifndef DSMR5READER_H
#define DSMR5READER_H

#define DSMR_OK               0
#define DSMR_NO_DATA          1
#define DSMR_INCORRECT_DATA   2
#define DSMR_NOT_DSMR         3
#define DSMR_NOT_DSMR_50      4

extern double dsmr_current[3];
extern char id[];

void dsmr5reader_init(void);
int dsmr5reader_check(void);
bool dsmr5reader_process_line(void);

#endif
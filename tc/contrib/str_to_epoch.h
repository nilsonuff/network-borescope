/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

char *string_to_epoch_err(void);
unsigned long string_to_epoch(char *str);

char *datetime_to_epoch_err(void);
unsigned long datetime_to_epoch(char *fmt, char *str);

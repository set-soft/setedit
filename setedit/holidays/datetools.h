#ifdef __cplusplus
extern "C" {
#endif

extern int  Day2Number(int d, int m, int y);
extern int  WeekDay(int d, int m, int y);
extern void Number2Day(int nNumber, int *day, int *month, int *year);
extern void Easter(int nY, int *nThursday, int *nFriday);

struct dayMonth
{
 int day,month;
 const char *description;
};

#ifdef __cplusplus
};

// This is exported to SETEdit, not available for the plug-ins
extern struct dayMonth *GetHolidays(int year, int &cant);
extern char *HolidaysGetLastError();
extern void  ConfigureHolidays();
extern void  CleanUpHolidays();
#endif


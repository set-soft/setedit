/*-------------------------------------------------------*/
/*                                                       */
/*   Turbo Vision 1.0                                    */
/*   Copyright (c) 1991 by Borland International         */
/*                                                       */
/*   Calendar.h: Header file for Calendar.cpp            */
/*-------------------------------------------------------*/

/* Modified by Salvador E. Tropea (SET) for SETEdit */

#if !defined( __CALENDAR_H )
#define __CALENDAR_H

class TCalendarView : public TView
{

public:

    TCalendarView(TRect & r);
    ~TCalendarView();
    virtual void handleEvent(TEvent& event);
    virtual void draw();
    char *getMonthStr(char *buffer, int size, int addArrows);
    
    static char upArrowChar;
    static char oupArrowChar;
    static char downArrowChar;
    static char odownArrowChar;

private:

    void updateYear();
    Boolean isHoliday(int day, int month);
    unsigned days, month, year;
    unsigned curDay, curMonth, curYear;
    stTVIntl *cNDays;
    // Holidays
    struct dayMonth *listOfHolidays;
    int              numOfHolidays;
};

class TCalendarWindow : public TWindow
{
public:
    TCalendarWindow();
    TCalendarView *view;
    virtual void handleEvent(TEvent& event);
};

#endif      // __CALENDAR_H

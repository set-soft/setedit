/*---------------------------------------------------------*/
/*                                                         */
/*   Turbo Vision 1.0                                      */
/*   Copyright (c) 1991 by Borland International           */
/*                                                         */
/*   Calendar.cpp:  TCalenderWindow member functions.      */
/*---------------------------------------------------------*/

/* Modified by Robert Hoehne to be used with RHIDE */
/* Modified by Salvador E. Tropea (SET) for SETEdit */

#define Uses_TRect
#define Uses_TEvent
#define Uses_TKeys
#define Uses_TDrawBuffer
#define Uses_TStreamableClass
#define Uses_TStreamable
#define Uses_TView
#define Uses_TWindow
#define Uses_string
#define Uses_AllocLocal
#define Uses_stdlib
#include <tv.h>

#include <ctype.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>

#include <calendar.h>


char TCalendarView::upArrowChar  ='\036';
char TCalendarView::oupArrowChar ='\036';
char TCalendarView::downArrowChar ='\037';
char TCalendarView::odownArrowChar='\037';

static char *monthNames[] = {
    "",
    __("January"),  __("February"), __("March"), __("April"), __("May"),
    __("June"), __("July"), __("August"), __("September"), __("October"),
    __("November"), __("December")
};


static unsigned char daysInMonth[] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};


#if 1
// This should be more portable:
static
void GetTime(time_t *tt)
{
 time(tt);
}
#else
// Funny that ftime is a BSD 4.2 function and doesn't exist for FreeBSD
// Here just in case is needed.
static
void GetTime(time_t *tt)
{
 struct timeb tb;
 ::ftime(&tb);
 *tt=tb.time;
}
#endif

//
// TCalendarView functions
//

TCalendarView::TCalendarView(TRect& r) : TView( r )
{
    struct tm *tm;
    time_t tt;

    options |= ofSelectable;
    eventMask |= evMouseAuto;

    GetTime(&tt);
    tm = localtime(&tt);
    year = curYear = 1900 + tm->tm_year;
    month = curMonth = tm->tm_mon + 1;
    curDay = tm->tm_mday;

    drawView();
}


unsigned dayOfWeek(unsigned day, unsigned month, unsigned year)
{
    int century, yr, dw;

    if(month < 3)
        {
        month += 10;
        --year;
        }
    else
        month -= 2;

    century = year / 100;
    yr = year % 100;
    dw = (((26 * (int)month - 2) / 10) + (int)day + yr + (yr / 4) + (century / 4) - 
                (2 * century)) % 7;

    if(dw < 0)
        dw += 7;

    return((unsigned)dw);
}

/**[txh]********************************************************************

  Description:
  It fills the passed buffer with the month and year currently displayed by
the calendar. The buffer should have size+1 bytes. (SET)

***************************************************************************/

char *TCalendarView::getMonthStr(char *buffer, int size, int addArrows)
{
 const char *monthName=_(monthNames[month]);
 int l=max(strlen(monthName),15)+5;
 if (addArrows)
    l+=2;

 AllocLocalStr(str,l+1);
 if (addArrows)
    sprintf(str,"%c%15s %4d%c", upArrowChar, monthName, year, downArrowChar);
 else
    sprintf(str,"%s %4d", monthName, year);

 strncpy(buffer,str,size);
 buffer[size]=0;
 return buffer;
}

void TCalendarView::draw()
{
    AllocLocalStr(str,size.x+1);
    unsigned current = 1 - dayOfWeek(1, month, year);
    unsigned days = daysInMonth[month] + ((year % 4 == 0 && month == 2) ? 1 : 0);
    char color, boldColor;
    int  i, j;
    TDrawBuffer buf;

    color = getColor(6);
    boldColor = getColor(7);

    buf.moveChar(0, ' ', color, size.x);

    getMonthStr(str,size.x,1);

    buf.moveStr(0, str, color);
    writeLine(0, 0, size.x, 1, buf);

    buf.moveChar(0, ' ', color, size.x);
    buf.moveStr(1, _("Su Mo Tu We Th Fr Sa"), color);
    writeLine(0, 1, size.x, 1, buf);

    for(i = 1; i <= 6; i++)
        {
        buf.moveChar(0, ' ', color, size.x);
        for(j = 0; j <= 6; j++)
            {
            if(current < 1 || current > days)
                buf.moveStr(1+j*3, "   ", color);
            else
                {
                sprintf(str, "%2d", (int)current);
                if(year == curYear && month == curMonth && current == curDay)
                    buf.moveStr(1+j*3, str, boldColor);
                else
                    buf.moveStr(1+j*3, str, color);
                }
            current++;
            }
        writeLine(0, i+1, size.x, 1, buf);
        }
}


void TCalendarView::handleEvent(TEvent& event)
{
    TPoint point;

    TView::handleEvent(event);
    if (state && sfSelected)
        {
        if ( (event.what & evMouse) && (evMouseDown || evMouseAuto) )
            {
            point = makeLocal(event.mouse.where);
            if (point.x == 0 && point.y == 0)
                {
                ++month;
                if (month > 12)
                    {
                    ++year;
                    month = 1;
                    }
                drawView();
                }
            else if (point.x == 21 && point.y == 0)
                {
                --month;
                if (month < 1)
                    {
                    --year;
                    month = 12;
                    }
                drawView();
                }
            }
        else if (event.what == evKeyboard)
            {
            if ( (loByte(event.keyDown.keyCode) == '+') ||
              event.keyDown.keyCode == kbDown)
                {
                ++month;
                if (month > 12)
                    {
                    ++year;
                    month = 1;
                    }
                }
            else if ( (loByte(event.keyDown.keyCode) == '-') ||
              event.keyDown.keyCode == kbUp)
                {
                --month;
                if (month < 1)
                    {
                    --year;
                    month = 12;
                    }
                }
            drawView();
            }
        }
}


//
// TCalendarWindow functions
//

TCalendarWindow::TCalendarWindow() :
    TWindow( TRect(1, 1, 25, 11), _("Calendar"), wnNoNumber ),
    TWindowInit( &TCalendarWindow::initFrame )
{
    TRect r(getExtent());

    flags &= ~(wfZoom | wfGrow);
    growMode = 0;

    palette = wpCyanWindow;

    r.grow(-1, -1);
    view = new TCalendarView( r );
    insert( view );
}

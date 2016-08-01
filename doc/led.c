
/*
 * led.c:
 *	Simple test program to see if we can drive a 7-segment LED
 *	display using the GPIO and little else on the Raspberry Pi
 *
 *	Copyright (c) 2012 Gordon Henderson
 ***********************************************************************
 *    This is is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

 */

#undef	PHOTO_HACK

#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>

#define	D1	10
#define	D2	11
#define	D3	12
#define	D4	13


// Segment mapping data

/*
 *	 --a--
 *	|     |
 *	f     b
 *	|     |
 *	 --g--
 *	|     |
 *	e     c
 *	|     |
 *	 --d--  p
 *
 *
 * connectin into the Pi as follows:
 *	Segments a,b,c,d,e,f,g,p -> wiringPi pins 0-7
 *			(or GPIO 17,18,21,22,23,24,25,4)
 *	The commons for Digits 1,2,3,3 -> wiringPi pins 10,11,12,13
 *			(or GPIO 8,7,,10,9)
 *	The commons have a 200 ohm resistor.
 *
 *	To light a segment:
 *		Set the common high and set the segment low.
 *		Only light ONE segment at a time. (We can light more
 *		but it will affect the brightness as there is only one
 *		resistor. We can move to using 8 resistors, but then the
 *		common pin would require a transistor to buffer the extra
 *		current required).
 */

static const uint8_t segmentDigits [] =
{
// a  b  c  d  e  f  g  p	Segments
// 0  1  2  3  4  5  6  7	wiringPi pin No.
   1, 1, 1, 1, 1, 1, 0, 0,	// 0
   0, 1, 1, 0, 0, 0, 0, 0,	// 1
   1, 1, 0, 1, 1, 0, 1, 0,	// 2
   1, 1, 1, 1, 0, 0, 1, 0,	// 3
   0, 1, 1, 0, 0, 1, 1, 0,	// 4
   1, 0, 1, 1, 0, 1, 1, 0,	// 5
   1, 0, 1, 1, 1, 1, 1, 0,	// 6
   1, 1, 1, 0, 0, 0, 0, 0,	// 7
   1, 1, 1, 1, 1, 1, 1, 0,	// 8
   1, 1, 1, 1, 0, 1, 1, 0,	// 9
   1, 1, 1, 0, 1, 1, 1, 0,	// A
   0, 0, 1, 1, 1, 1, 1, 0,	// b
   1, 0, 0, 1, 1, 1, 0, 0,	// C
   0, 1, 1, 1, 1, 0, 1, 0,	// d
   1, 0, 0, 1, 1, 1, 1, 0,	// E
   1, 0, 0, 0, 1, 1, 1, 0,	// F
   0, 0, 0, 0, 0, 0, 0, 0,	// blank
} ;
 

// digits:
//	A global variable which is written to by the main program and
//	read from by the thread that updates the display. Only the first
//	4 digits (ascii) are used.

char digits [8] ;


/*
 * displayDigits:
 *	This is our thread that's run concurrently with the main program.
 *	Essentially sit in a loop, parsing and displaying the data held in
 *	the "digits" global.
 *********************************************************************************
 */

void *displayDigits (void *dummy)
{
  uint8_t digit, segment ;
  uint8_t index, d, segVal ;

  for (;;)
  {
    for (digit = 0 ; digit < 4 ; ++digit)
    {
      digitalWrite (D1 + digit, 1) ;
      for (segment = 0 ; segment < 8 ; ++segment)
      {
	d = toupper (digits [digit]) ;
	/**/ if ((d >= '0') && (d <= '9'))	// Digit
	  index = d - '0' ;
	else if ((d >= 'A') && (d <= 'F'))	// Hex
	  index = d - 'A' + 10 ;
	else
	  index = 16 ;				// Blank

	segVal = segmentDigits [index * 8 + segment] ;

	digitalWrite (segment, !segVal) ;
	delayMicroseconds (100) ;
	digitalWrite (segment, 1) ;
      }
      digitalWrite (D1 + digit, 0) ;
    }
  }
}


/*
 * setHiPri:
 *	Attempmt to set our schedulling priority to something "high"
 *	and "real time".
 *	Need to be root to do this.
 *********************************************************************************
 */

void setHighPri (void)
{
  struct sched_param sched ;

  memset (&sched, 0, sizeof(sched)) ;

  sched.sched_priority = 10 ;
  if (sched_setscheduler (0, SCHED_RR, &sched))
    printf ("Warning: Unable to set high priority\n") ;
}


/*
 * setup:
 *	Initialise the hardware and start the thread
 *********************************************************************************
 */

void setup (void)
{
  int i, c ;
  pthread_t displayThread ;


  if (wiringPiSetup () == -1)
    exit (1) ;

  setHighPri () ;

// 8 segments

  for (i = 0 ; i < 8 ; ++i)
  {
    digitalWrite (i, 0) ;
    pinMode      (i, OUTPUT) ;
  }

// 4 commons

  for (i = 10 ; i < 13 ; ++i)
  {
    digitalWrite (i, 0) ;
    pinMode      (i, OUTPUT) ;
  }

  strcpy (digits, "    ") ;

  if (pthread_create (&displayThread, NULL, displayDigits, NULL) != 0)
  {
    printf ("thread create failed: %s\n", strerror (errno)) ;
    exit (1) ;
  }
  delay (10) ; // Just to make sure it's started

// Quick countdown LED test sort of thing

  c = 9999 ;
  for (i = 0 ; i < 10 ; ++i)
  {
    sprintf (digits, "%04d", c) ;
    delay (200) ;
    c -= 1111 ;
  }

#ifdef PHOTO_HACK
  sprintf (digits, "%s", "1234") ;
  for (;;)
    delay (1000) ;
#endif

}


/*
 * teenager:
 *	No explanation needed. (Nor one given!)
 *********************************************************************************
 */

void teenager (void)
{
  char *message = "    b00b1e5    babe    cafe    b00b" ;
  int i ;

  for (i = 0 ; i < strlen (message) - 3 ; ++i)
  {
    strncpy (digits, &message [i], 4) ;
    delay (500) ;
  }
  delay (1000) ;
  for (i = 0 ; i < 3 ; ++i)
  {
    strcpy (digits, "    ") ;
    delay (150) ;
    strcpy (digits, "b00b") ;
    delay (250) ;
  }
  delay (1000) ;
  strcpy (digits, "    ") ;
  delay (1000) ;
}


/*
 *********************************************************************************
 * main:
 *	Let the fun begin
 *********************************************************************************
 */

int main (void)
{
  struct tm *t ;
  time_t     tim ;

  setup    () ;
  teenager () ;

  for (;;)
  {
    tim = time (NULL) ;
    t   = localtime (&tim) ;

    sprintf (digits, "%02d%02d", t->tm_min, t->tm_sec) ;

    delay (500) ;
  }

  return 0 ;
}

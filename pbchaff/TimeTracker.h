/* Written by Tristan Smith */

#ifndef _TIME_TRACKER_H
#define _TIME_TRACKER_H

#include <climits>

/* This class will take advantage of clock() to calculate the 
   number of seconds that have passed since the beginning of 
   the program.  It is brittle because we have to use this 
   particular implementation of ULONG_MAX and assume that the clock()
   function wraps at the same value.

   NOTE:  The function updateTime should be run at least once an hour 
   to avoid the possibility that more than one wrap occur before
   being detected.
   
   NOTE:  m_cumulativeSeconds probably only needs to be an int but 
   I made it unsigned long just to be careful.

   IMPROVEMENT:  Clean this up by creating a .cpp file.
   IMPROVEMENT:  Also check for the errno setting if I get a -1 value   
   with the clock() call.(#include <cerrno>).  I looked but couldn't 
   find what this setting would be...   
*/

// This constant is what makes this brittle:
static const unsigned long int k_ulongMax = ULONG_MAX;

class TimeTracker
{
public:
    TimeTracker() { m_lastTime = 0; }
    void updateTime();
    void initialize();
    double getElapsedTime();
        
private:
    unsigned long int m_lastTime;
    unsigned long int m_cumulativeClocks;
    unsigned long int  m_cumulativeSeconds;
};

#endif // _TIME_TRACKER_H













/* Written by Tristan Smith */

#include "TimeTracker.h"
#include <ctime>


/* This updates by calcuting the difference between m_lastTime and
   the time now.  It must also check to see if the clock() function
   has wrapped and do appropriate calculations if so.

   NOTE:  Notice taht m_lastTime is never rounded.  Instead, when we
   divide by CLOCKS_PER_SEC to find cumulative seconds, we store the
   remainder so that accuracy can be maintained.
*/

void TimeTracker::updateTime()
{
    // this is because I didn't know what I was doing(the long double!):
    long double temp = clock();
    if ( temp != - 1 ) 
    {
        unsigned long int timeNow = (unsigned long int) temp;
        unsigned long int cum = 0;
        if ( timeNow < m_lastTime )
        {
            cum = ( k_ulongMax - m_lastTime );
            cum += timeNow;
        }
        else 
        {
            cum = ( timeNow - m_lastTime );
        }
        m_cumulativeSeconds += ( cum / CLOCKS_PER_SEC );//add whole seconds
        m_cumulativeClocks += ( cum % CLOCKS_PER_SEC ); //don't lose piece
        m_cumulativeSeconds += ( m_cumulativeClocks / CLOCKS_PER_SEC );
        m_cumulativeClocks = ( m_cumulativeClocks % CLOCKS_PER_SEC );
        m_lastTime = timeNow;
    }
}

// Restart the clock.
void TimeTracker::initialize()
{
    m_cumulativeSeconds = 0;
    m_cumulativeClocks = 0;
    m_lastTime = ( unsigned long int ) clock();
}

double TimeTracker::getElapsedTime()
{
    updateTime();
    return double(m_cumulativeSeconds) + double(m_cumulativeClocks) / CLOCKS_PER_SEC;
}



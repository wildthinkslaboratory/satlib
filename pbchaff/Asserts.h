/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#ifndef _ASSERTS_H
#define _ASSERTS_H

#ifdef BASE_ASSERTS

#define ASSERT(expr) \
if (expr) {} else failure(__FILE__,__LINE__,#expr)
#else

#define ASSERT(expr) \
{}

#endif

#endif

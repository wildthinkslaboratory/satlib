/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#ifndef   _UTILITIES_H
#define   _UTILITIES_H

#include <iostream>
#include <cstdlib>
#include "Asserts.h"

typedef int ClauseID;

enum WhichSet {
  LAZY,
  PB,
  MOD2
};

static inline void fatalError(char * message)
{
  std::cerr << "FATAL ERROR: " << message << std::endl;
  exit(1);
}

static inline void failure(char *file,int line,char *expression)
{
  std::cerr << "FAILURE: Expression " << expression << " failed in file "
	    << file << " at line "
	    << line << "." << std::endl;
  exit(1);
}

static inline void warning(char *file,int line,char *expression)
{
  std::cerr << "WARNING: Expression " << expression << " failed in file "
	    << file << " at line "
	    << line << "." << std::endl;
}

static inline int coinFlip() { return rand() & 1; }


/* greatest common denominator */
static inline int gcd(int x, int y)
{
  int lo,hi,rem;

  if (y == 1) return 1;
  if (x < y) { lo = x; hi = y; }
  else { lo = y; hi = x; }
  // error lo == 0
  ASSERT(lo != 0);
  while ((rem = hi % lo))
    {
      hi = lo;
      lo = rem;
    }
  return lo;
}

/* an approximation of number r elements subsets of an n element group */
static inline int choose(int n, int r)
{
  if (r <= 0 || n <= 0) return 1;

  double result = 1.0;
  double num = (double) (n);
  double den = (double) (r);
  
  for (size_t i=r; i >= 1; --i)
    {
      result *= num;
      result /= den;
      num -= 1.0;
      den -= 1.0;
    }
  return (int) (result);
}




#endif


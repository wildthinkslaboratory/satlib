/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#ifndef _RATIONAL_H
#define _RATIONAL_H
#include <iostream>
#include <cstdlib>


/*
  CLASS: Rational
  
  PURPOSE: class for expressing rational numbers

*/
class Rational {
 protected:
  int m_numerator;
  int m_denominator;

 public:
	     
  Rational(int n, int d) : m_numerator(n), m_denominator(d) { ASSERT(d != 0); }

  /******************* OPERATOR OVERLOAD *************************/
  
  bool operator<(const int i) const { return (m_numerator < (m_denominator * i)); }
  bool operator<(const Rational &r) const
    {
      return ((m_numerator * r.m_denominator) < (m_denominator * r.m_numerator));
    }
  bool operator>(const Rational &r) const
    {
      return ((m_numerator * r.m_denominator) > (m_denominator * r.m_numerator));
    }
  bool operator<=(const int i) const { return (m_numerator <= (m_denominator * i)); }
  bool operator>(const int i) const { return (m_numerator > (m_denominator * i)); }
  bool operator!=(const int i) const { return (m_numerator != (m_denominator * i)); }
  void operator *=(const int i) { m_numerator *= i; }
  void operator *=(const Rational &r)
  {
    ASSERT(r.m_denominator != 0);
    m_numerator *= r.m_numerator;
    m_denominator *= r.m_denominator;
  }
  void operator /=(const Rational &r)
  {
    ASSERT(r.m_numerator != 0);
    m_numerator *= r.m_denominator;
    m_denominator *= r.m_numerator;
  }

  /******************** READ ************************/
  
  Rational absoluteValue() const { return Rational(abs(m_numerator),abs(m_denominator)); }
  int ceiling() const
    {
      int remainder = m_numerator % m_denominator;
      int value = m_numerator / m_denominator;
      return (remainder > 0) ? ++value : value;
    }

  void print(std::ostream& os = std::cout) const
    {
      os << m_numerator << "/" << m_denominator << " " << std::flush;
    }

  /******************** WRITE ************************/
  
  void setValue(int n, int d)
    {
      ASSERT(d != 0);
      m_numerator = n; m_denominator = d;
    }

  
};

inline std::ostream& operator<<(std::ostream& os, const Rational& r) {
  r.print(os);
  return os;
}



#endif

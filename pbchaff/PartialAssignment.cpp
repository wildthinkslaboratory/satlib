/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#include "PartialAssignment.h"
#include <iostream>

using namespace std;

void PartialAssignment::print(ostream& os) const
{
  os << "Partial Assignment: ";
  for (size_t i=0; i < m_size; i++)
    {
      Literal l = getLiteral(i);
      if (!m_reasons[l.getAtom()]) cout << endl << "branch  ";
      os << l << "  ";
    }
  os << endl;
}


void PartialAssignment::printListForm(ostream& os) const
{
  os << "Assignment: ";
  for (size_t i=0; i < m_size; i++)
    {
      os << (getSign(i) ? "" : "-") << getAtom(i) << " ";
    }
  os << endl;
}

void PartialAssignment::printNameForm(ostream& os) 
{
  os << "Partial Assignment: ";
  for (size_t i=0; i < m_size; i++)
    {
      Literal l = getLiteral(i);
      //if (!m_reasons[l.getAtom()]) cout << endl << "branch  ";
      if (l.getSign()) cout << lookup(l.getAtom()) << "  ";
    }
  os << endl;
}

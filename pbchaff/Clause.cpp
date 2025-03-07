/**********************************/
/** written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#include "Clause.h"
#include <iostream>
using namespace std;




void Clause::print(ostream& os) const
{
  if (!inUse()) os << "\t\t\tremoved constraint";
  for (size_t i = 0; i < size(); ++i)
    getReadLiteral(i).print(os);
  os << " >= " << m_required << endl;
}

void Clause::printDetail(ostream& os) const
{
  if (!inUse()) os << "\t\t\tremoved constraint";
  for (size_t i = 0; i < size(); ++i)
    getReadLiteral(i).printDetail(os);
  os << " >= " << m_required << endl;
}


void PBClause::print(ostream& os) const
{
  if (!inUse()) os << "\t\t\tremoved constraint";
  for (size_t i = 0; i < size(); ++i)
    getReadLiteral(i).print(os);
  os << " >= " << m_required << endl;
}

void PBClause::printDetail(ostream& os) const
{
  if (!inUse()) os << "\t\t\tremoved constraint";
  for (size_t i = 0; i < size(); ++i)
    getReadLiteral(i).printDetail(os);
  os << " >= " << m_required << endl;
}

void Mod2Clause::print(ostream& os) const
{
  if (!inUse()) os << "\t\t\tremoved constraint";
  os << "( ";
  for (size_t i = 0; i < size(); ++i)
    getReadLiteral(i).print(os);
  os << " )mod2 = " << m_sumMod2 << endl;
}

void Mod2Clause::printDetail(ostream& os) const
{
  if (!inUse()) os << "\t\t\tremoved constraint";
  os << "( ";
  for (size_t i = 0; i < size(); ++i)
    getReadLiteral(i).printDetail(os);
  os << " )mod2 = " << m_sumMod2 << endl;
}




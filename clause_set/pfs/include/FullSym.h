#ifndef __FULL_SYM__
#define __FULL_SYM__

#include <iomanip>
#include <string>
#include <set>
#include "common.h"
#include "Assignment.h"
#include "PfsCounter.h"
using namespace std;

namespace zap
{

typedef vector<Variable> FSOrbit;


class FullSym : public vector<FSOrbit>
{
  string                m_name;       // the predicate_argument name
  string                m_domain;     // the domain moved
  vector<bool>          m_atom_in;    // is a particular ground variable in the symmetry
  vector<pair<Row,Column> > m_atom_index;  // how to find the position of an atom
  
public:

  FullSym() { }
  FullSym(const vector<FSOrbit>& o, const string& n = string(), const string& d = string());
  
  size_t    orbit_size        () const { return (*this)[0].size(); }
  string    name              () const { return m_name; }
  string    domain            () const { return m_domain; }

  Column           column           (Literal l) const;  
  bool             contains         (Variable a) const { return (a < m_atom_in.size() && m_atom_in[a]); }
  Literal          image            (Literal l,const vector<Column>& before,const vector<Column>& after) const;
  vector<Literal>  orbit            (Literal l, const set<Column>& c) const;
  size_t           max_id           () const { return m_atom_in.size() - 1; }

  set<Column>     point_stabilizer (const vector<Literal>& points) const;
  set<Column>     clause_stabilizer(const vector<Literal>& v) const;
  friend ostream& operator << (ostream& os, const FullSym& s); 
  
  //////////////////////////////  REVIEW  ////////////////////////////////////////////
  //   bool      hits(const vector<Literal>& u) const;             // used in resolution, could replace with fixes
  //   set<Column> set_stabilize(const vector<Literal>& v) const;  // used in resolution, very wrong
  void print_assignment(const Assignment& PA, ostream& os = cout) const;
  
  // vector<set<Column> > set_stabilizer(const Assignment& P, const set<Column>& cols) const;
  

};


inline Literal FullSym::image(Literal l,const vector<Column>& before,const vector<Column>& after) const
{
  if (!contains(l.variable())) return l;
  pair<Row,Column> r_c = m_atom_index[l.variable()];
  Column c_image = column_image(r_c.second,before,after);
  if (c_image == r_c.second) return l;
  return Literal((*this)[r_c.first][c_image],l.sign());
}






inline ostream& operator<< (ostream& os, const FullSym& s)
{
  os << s.name() << " : " << s.domain() << " [" << endl;
  size_t max = s[s.size()-1][s.orbit_size()-1];
  size_t width = (size_t)(logint(max,10)) + 3;
  os << "\t";
  for (size_t i=0; i < s.orbit_size(); i++)
    os << setw(width) << right << i;
  os << endl << "\t";
  for (size_t i=0; i < s.orbit_size(); i++) 
    for (size_t j=0; j < (width); j++) os << '-';
  os << endl;
  for (size_t i = 0 ; i < s.size() ; ++i) {
    os << "\t";
    for (size_t j = 0 ; j < s[i].size() ; ++j)
      os << setw(width) << right << s[i][j];
    os << endl;
  }
  os << ']' << endl;
  return os;
}


inline void FullSym::print_assignment(const Assignment& PA, ostream& os) const
{
  os << name() << " : " << domain() << " [" << endl;
  size_t max = (*this)[size()-1][orbit_size()-1];
  size_t width = (size_t)(logint(max,10)) + 3;
  os << "\t";
  for (size_t i=0; i < orbit_size(); i++)
    os << setw(width) << right << i;
  os << endl << "\t";
  for (size_t i=0; i < orbit_size(); i++) 
    for (size_t j=0; j < (width); j++) os << '-';
  os << endl;
  
  for (size_t i = 0 ; i < size() ; ++i) {
    os << "\t";
    for (size_t j = 0 ; j < (*this)[i].size() ; ++j) {
      size_t value = PA.value((*this)[i][j]);
      if (value == UNKNOWN)
		os << setw(width) << right << '*';
      else
		os << setw(width) << right << (value ? '|' : '-');
    }
    os << endl;
  }
  os << ']' << endl;
}




} // end namespace zap
#endif











#include "FullSym.h"
#include "Set.h"
#include <map>

namespace zap
{

FullSym::FullSym(const vector<FSOrbit>& o, const string& n, const string& d) :
  vector<FSOrbit>(o), m_name(n), m_domain(d)
{
  size_t max_atom = 0;
  for (size_t i=0; i < size(); i++)
    for (size_t j=0; j < orbit_size(); j++)
      if ((*this)[i][j] > max_atom) max_atom = (*this)[i][j];
  m_atom_index = vector<pair<Row,Column> >(max_atom+1);
  m_atom_in = vector<bool>(max_atom+1,false);
  
  for (size_t i=0; i < size(); i++) {
    for (size_t j=0; j < orbit_size(); j++) {
      Variable atom = (*this)[i][j];
      m_atom_index[atom] = make_pair(i,j);
      m_atom_in[atom] = true;
    }
  }
}

Column FullSym::column(Literal l) const
{
  if (contains(l.variable())) return m_atom_index[l.variable()].second;
  return 0;
}





vector<Literal> FullSym::orbit(Literal l, const set<Column>& c ) const
{
  if (!contains(l.variable())) return vector<Literal>();
  pair<Row,Column> r_c = m_atom_index[l.variable()];
  
  if (c.find(r_c.second) == c.end()) return vector<Literal>();
  vector<Literal> answer;
  set<Column>::iterator it = c.begin();
  for ( ; it != c.end(); it++) {
    answer.push_back(Literal((*this)[r_c.first][*it],l.sign()));
  }
  return answer;
}


// vector<set<Column> > FullSym::set_stabilizer(const Assignment& P, const set<Column>& c) const
// {

//   if (c.size() <= 1) return vector<set<Column> >();
  
//   set<vector<size_t> > ordered_columns;
//   set<Column>::const_iterator it = c.begin();
//   for ( ; it != c.end(); it++) {
// 	vector<size_t> column(size()+1,0);
// 	for (size_t i=0; i < size(); i++) {
// 	  column[i] = P.value((*this)[i][*it]);
// 	}
// 	column[size()] = *it;
// 	ordered_columns.insert(column);
//   }

//   set<vector<size_t> >::const_iterator it1 = ordered_columns.begin();
//   set<vector<size_t> >::const_iterator it2 = it1;
//   ++it2;
//   set<Column> a;
//   a.insert((*it1)[size()]);
//   vector<set<Column> > answer;
//   for ( ; it2 != ordered_columns.end(); it1++, it2++) {
// 	vector<size_t> current, next;
// 	for (size_t i=0; i < size(); i++) {
// 	  current.push_back((*it1)[i]);
// 	  next.push_back((*it2)[i]);
// 	}
// 	if (current == next) {
// 	  a.insert((*it2)[size()]);
// 	}
// 	else {
// 	  if (a.size() > 1) answer.push_back(a);
// 	  a.clear();
// 	  a.insert((*it2)[size()]);
// 	}
//   }
//   if (a.size() > 1) answer.push_back(a);
//   return answer;
// }



set<Column> FullSym::point_stabilizer(const vector<Literal>& points) const
{
  Set<Column> fixed;
  for (size_t i=0; i < points.size(); i++) {
    if (contains(points[i].variable()))
      fixed.insert(m_atom_index[points[i].variable()].second);
  }
  
  set<Column> answer;
  for (size_t i=0; i < orbit_size(); i++) 
    if (!fixed.contains(i)) answer.insert(i);

  return answer;
}

set<Column> FullSym::clause_stabilizer(const vector<Literal>& c) const
{
  map<Row,set<Column> > moving_columns;
  Set<Literal> cset;
  for (size_t i=0; i < c.size(); i++) cset.insert(c[i]);

  while (!cset.empty()) {
	Variable v = cset.begin()->variable();
	bool sign = cset.begin()->sign();
    if (!contains(v)) {
      cset.pop();
      continue;
	}

    Row row = m_atom_index[v].first;
	set<Column> columns;

	for (size_t j=0; j < orbit_size(); j++) {
	  set<Literal>::iterator in = cset.find(Literal((*this)[row][j],sign));  
      if (in != cset.end()) {
		columns.insert(j);
		cset.erase(in);
	  }
	}

	moving_columns[row] = columns;
  }

  // if there are multiple rows moving they have to agree on columns
  if (moving_columns.size() == 0) return set<Column>();
  if (moving_columns.size() == 1) return moving_columns.begin()->second;
  if (moving_columns.size() > 1) {
	quit("In clause stabilizer and can't do this yet.");
  }
  return set<Column>();
}


// // this is sort of like fixes?
// bool FullSym::hits(const vector<Literal>& u) const
// {
//   for (size_t i=0; i < u.size(); i++) {
//     Variable atom = get_atom(u[i]);
//     if (contains(atom)) return true;
//   }
//   return false;
// }

} // end namespace zap

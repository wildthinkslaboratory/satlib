#ifndef __GLOBAL_PRODUCT_GROUP__
#define __GLOBAL_PRODUCT_GROUP__
#include "FullSym.h"
#include <map>

using namespace std;


namespace zap
{
class GlobalProductGroup : public vector<FullSym>
{
  mutable map<string,size_t> m_name_to_id;  // predicate_argument name to index, used by group builder
  vector<size_t>             m_predicate_ranges; // the range of variable ids for each predicate
  vector<size_t>             m_predicate_map; // the predicate moved by each FullSym
 public:

  //////////////////////////  FINISHED  //////////////////////////////////
  GlobalProductGroup(unsigned n = 0) : vector<FullSym>(n) { }
  
  size_t              add_symmetry    (FullSym s);
  size_t              index           (const string& name) const;
  size_t              number_variables() const;
  vector<Literal>     get_all_literals() const;

  friend ostream& operator<< (ostream& os, const GlobalProductGroup& sg);
  void print_assignment(const Assignment& P, ostream& os = cout) const; 
  
  ////////////////////////////  REVIEW  ///////////////////////////////////
  
//   // used in the resolution code.  Review when I review resolution
//   bool hits(size_t i,const vector<Literal>& u) const { return (*this)[i].hits(u); }

//   // keep but rewrite, add point stabilizer 
//   ProductSubgroup set_stabilize(const vector<Literal>& v) const ;
//   ProductSubgroup multi_stabilize(const vector<vector<Literal> >& v) const ;
  
//   // used in PGTransportVector() TransportVector(), will disappear I think
//   vector<size_t> get_domain_point(const vector<size_t>& indexes, Literal l) const;

};


/////////////////////////  INLINES  ////////////////////////////////////////

inline void GlobalProductGroup::print_assignment(const Assignment& P, ostream& os) const
{
  for (size_t i=0; i < size(); i++) {
	operator[](i).print_assignment(P,os);
	os << endl;
  }
}


inline size_t GlobalProductGroup::index(const string& name) const
{
  map<string,size_t>::iterator it = m_name_to_id.find(name);
  if (it == m_name_to_id.end()) quit("argument name not found in GlobalProductGroup::index");
  return it->second;
}

inline size_t GlobalProductGroup::number_variables() const
{
  return global_vars.atom_name_map.size();
//   size_t max_id = 0;
//   for (size_t i=0; i < size(); i++)
// 	if (operator[](i).max_id() > max_id) max_id = operator[](i).max_id();
//   return max_id;
}

inline size_t GlobalProductGroup::add_symmetry(FullSym s)
{
  push_back(s);
  m_predicate_map.push_back(0);
  size_t i=0;
  for (; i < m_predicate_ranges.size(); i++)
	if (s[s.size()-1][s[s.size()-1].size()-1] == m_predicate_ranges[i]) break;
  if (i == m_predicate_ranges.size())
	m_predicate_ranges.push_back(s[s.size()-1][s[s.size()-1].size()-1]);
  m_predicate_map[size()-1] = i;

  return m_name_to_id[s.name()] = size() - 1;
}

inline ostream& operator<< (ostream& os, const GlobalProductGroup& sg)
{
  for (size_t i = 0 ; i < sg.size() ; ++i) os << (i ? " x " : "") << sg[i];
  os << endl;
  return os;
}

inline vector<Literal> GlobalProductGroup::get_all_literals() const
{
  Variable first_var = 0;
  vector<Literal> answer;
  for (size_t i=0; i < size(); i++) {
	if (operator[](i)[0][0] != first_var) {
	  first_var = operator[](i)[0][0];
	  const FullSym& fs = operator[](i);
	  for (size_t j=0; j < fs.size(); j++)
		for (size_t k=0; k < fs[j].size(); k++)
		  answer.push_back(Literal(fs[j][k],true));
	}
  }
  return answer;
}


} // end namespace zap
#endif

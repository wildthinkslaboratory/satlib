#ifndef __ACTION__
#define __ACTION__

#include "common.h"
#include "PfsCounter.h"
#include "GlobalProductGroup.h"
namespace zap
{

class PfsCounter
{
  vector<size_t>          m_current;
  vector<bool>            m_in;
  const vector<Column>*   m_columns;
  vector<size_t>          m_before;
  vector<size_t>          m_after;
  
  bool increment_index(size_t index);
  bool lowest_available(size_t index);
public:
  PfsCounter() { }
  PfsCounter(size_t subset_size, size_t max_value,const vector<Column>* cols);
  size_t size() const { return m_current.size(); }
//   vector<Column> current_rep() const;
  const vector<Column>& before() const { return m_before; }
  const vector<Column>& after()  const { return m_after; }
  bool next();
//   friend ostream& operator << (ostream& os, const PfsCounter& c);
};


///////////////////////////////  INLINES  ///////////////////////////////////////////

inline PfsCounter::PfsCounter(size_t subset_size, size_t max_value, const vector<Column>* cols) :
  m_current(vector<size_t>(subset_size,0)),
  m_in(vector<bool>(max_value,false)),
  m_columns(cols),
  m_before(subset_size,0),
  m_after(subset_size,0)
{
  for (size_t i=0; i < subset_size; i++) {
    m_current[i] = i;
    m_in[i] = true;
  }
}



inline bool PfsCounter::increment_index(size_t index) {
 
  size_t i = m_current[index] + 1;
  while (i < m_in.size() && m_in[i]) i++;
  if (i == m_in.size()) return false;
  
  m_in[m_current[index]] = false;
  m_in[i] = true;
  m_current[index] = i;
  return true;
}

inline bool PfsCounter::lowest_available(size_t index) {
 
  size_t i = 0;
  while (i < m_in.size() && m_in[i]) i++;
  if (i == m_in.size()) return false;
  
  m_in[i] = true;
  m_current[index] = i;
  return true;
}

inline bool PfsCounter::next() 
{
  if (m_current.empty()) return false;

  
  for (size_t i=0; i < m_current.size(); i++) m_before[i] = (*m_columns)[m_current[i]];
  
  size_t index = size() - 1;
  while (!increment_index(index)) {
    if (index == 0) return false;
    m_in[m_current[index]] = false;
    --index;
  }  
  for (size_t i=index+1; i < size(); i++) lowest_available(i);

  
  for (size_t i=0; i < m_current.size(); i++) m_after[i] = (*m_columns)[m_current[i]];
  
  return true;
}

class IndexVector : public vector<bool>
{
  public:
  IndexVector() { }
  IndexVector(size_t sz, bool v) : vector<bool>(sz,v) { }
  //  IndexVector operator*(const IndexVector& v) const;
  IndexVector operator+(const IndexVector& v) const;
  bool operator<(const IndexVector& v) const;
  size_t dot_product(const IndexVector& v) const;
  bool zero() const;
};

  
class Action
{ // this is no longer a struct, this data needs to be made private
public:
  const GlobalProductGroup*       parent_group;  /// these consts should protect the contents of pg
  vector<size_t>                  indexes;       /// which parts of the Global Group we are moving
  IndexVector                     index_vector;
  vector<set<Column> >            columns;       // which columns we're allowed to move
  vector<int>                     column_map;

  set<Column>     get_set_columns(Literal l) const;  // this could be private
  Action() : parent_group(0) { }
  Action(const vector<size_t>& n, const set<Column>& c, const GlobalProductGroup* gg);
  Action(const IndexVector& iv, const vector<set<Column> >& c, const GlobalProductGroup* gg);
  
  bool             empty() const { return (!indexes.size()) || (!columns.size()); }
  vector<Literal>  orbit(Literal point) const;
  vector<Column>   get_columns(Literal l) const;
  vector<Column>   get_columns(const vector<Literal>& c) const;
  Literal          image(Literal l, const PfsCounter& counter) const;
  Literal          image(Literal l, const vector<Column>& before, const vector<Column>& after) const;
  bool             fixes(Literal l,const vector<Column>& c) const;
  bool             fixes(Literal l) const;
  bool             moves(vector<Literal>& c) const;
  bool             stabilizes_set(set<Literal>& s) const;
  Action           set_stabilizer(const Assignment& P) const;
  bool             independent(const Action& a) const;
  
  friend ostream& operator << (ostream& os, const Action& a);

};


//////////////////////////////////  INLINES  ////////////////////////////////////////

inline bool IndexVector::operator<(const IndexVector& v) const {
  if (size() != v.size()) return false;
  for (size_t i=0; i < size(); i++) {
	if (operator[](i) == v[i]) continue;
	return operator[](i) > v[i];
  }
  return false;
}

inline bool IndexVector::zero() const
{
  for (size_t i=0; i < size(); i++)
	if (operator[](i)) return false;
  return true;
}
	
// inline IndexVector IndexVector::operator*(const IndexVector& v) const
// {
//   if (size() != v.size()) return IndexVector();

//   IndexVector answer(size(),false);
//   for (size_t i=0; i < size(); i++) {
// 	answer[i] = operator[](i) && v[i];
//   }
//   return answer;
// }

inline IndexVector IndexVector::operator+(const IndexVector& v) const
{
  if (size() != v.size()) return IndexVector();
  
  IndexVector answer(size(),false);
  for (size_t i=0; i < size(); i++) {
	answer[i] = operator[](i) || v[i];
  }
  return answer;
}


inline size_t IndexVector::dot_product(const IndexVector& v) const
{
  if (size() != v.size()) return 0;

  size_t answer = 0;
  for (size_t i=0; i < size(); i++) {
	answer += operator[](i) && v[i];
  }
  return answer;
}

inline Action Action::set_stabilizer(const Assignment& P) const
{
  return Action();
}


inline Literal Action::image(Literal l, const PfsCounter& counter) const
{
  Literal im = l;
  for (size_t i=0; i < indexes.size(); i++)
    im = (*parent_group)[indexes[i]].image(im,counter.before(),counter.after());
  return im;
}

inline Literal Action::image(Literal l, const vector<Column>& before, const vector<Column>& after) const
{
  Literal im = l;
  for (size_t i=0; i < indexes.size(); i++)
    im = (*parent_group)[indexes[i]].image(im,before,after);
  return im;
}

inline bool Action::independent(const Action& a) const
{
  return (index_vector.dot_product(a.index_vector) == 0);
}


inline set<Column> Action::get_set_columns(Literal l) const
{
  set<Column> answer;
  for (size_t i=0; i < indexes.size(); i++) {
    if (parent_group->operator[](indexes[i]).contains(l.variable()))
	  answer.insert(parent_group->operator[](indexes[i]).column(l));
  }
  return answer;
}



} // end namespace zap
#endif

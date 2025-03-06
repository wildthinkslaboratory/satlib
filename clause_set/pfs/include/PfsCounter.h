#ifndef __COUNTER__
#define __COUNTER__

#include "common.h"

namespace zap
{

/*
  GOALS :
  --  Interface entirely through Columns.  Only the inner counter has to know how the counting is
      implemented.
  --  The ability to update an image as you increment.  Hide this from outside.
 */
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


// inline vector<Column> PfsCounter::current_rep() const
// {
//   vector<Column> rep(m_current.size());
//   for (size_t i=0; i < m_current.size(); i++)
//     rep[i] = (*m_columns)[m_current[i]];
//   return rep;
// }

// inline ostream& operator << (ostream& os, const PfsCounter& c)
// {
//   for (size_t i=0; i < c.size(); i++) os << c.m_current[i] << ' ';
//   return os;
// }

} // end namespace zap

#endif

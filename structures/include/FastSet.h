#ifndef _FAST_SET_H
#define _FAST_SET_H

#include <vector>
using namespace std;

namespace zap
{

class FastSet
{
protected:
  vector<bool> m_in;
  vector<size_t> m_position;
  vector<size_t> m_elements;
  
  void resize(size_t sz);
  
public:
  FastSet() { }
  FastSet(size_t nv) : m_in(vector<bool>(nv+1,false)), m_position(vector<size_t>(nv,0)) { }
  
  size_t  size() const { return m_elements.size(); }          // constant time
  bool    empty() const { return m_elements.size() == 0; }    // constant time
  bool    contains(size_t e) const;                           // constant time
  
  size_t  pop();                                              // constant time
  void    insert(size_t v);                                   // constant time
  void    remove(size_t v);                                   // constant time
  void    clear();
  void    intersection(const FastSet& s, FastSet& result) const;

  size_t operator[](size_t i) const;
};

inline bool FastSet::contains(size_t e) const
{
  if (e >= m_in.size()) return false;
  return m_in[e]; 
}


inline size_t FastSet::pop()
{
  size_t back = m_elements.back();
  remove(back);
  return back;
}

inline void FastSet::insert(size_t e)
{
  if (e >= m_in.size()) resize(e);
  if (m_in[e] != true) {
    m_in[e] = true;
    m_position[e] = m_elements.size();
    m_elements.push_back(e);
  }
}

inline void FastSet::remove(size_t e)
{
  if (e >= m_in.size() || !m_in[e]) return;
  m_in[e] = false;
  size_t b = m_elements.back();
  m_elements[m_position[e]] = b;
  m_elements.pop_back();
  m_position[b] = m_position[e];
 
}

inline void FastSet::resize(size_t sz)
{
  size_t more = sz + 1 - m_in.size();
  m_in.insert(m_in.end(),more,false);
  m_position.insert(m_position.end(),more,0);
}

inline void FastSet::clear()
{
  while (!empty()) pop();
}

inline size_t FastSet::operator[](size_t i) const { return m_elements[i]; }

inline void FastSet::intersection(const FastSet& s, FastSet& result) const
{
  for (size_t i=0; i < size(); i++)
	if (s.contains(operator[](i))) result.insert(operator[](i));
}



} // end namespace zap
#endif

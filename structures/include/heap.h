#ifndef _HEAP_H
#define _HEAP_H

#include <iostream>
#include "common.h"

namespace zap
{

////////////////////////////////////////   HEAP   ////////////////////////////////////////

/*  This data structure is used for the unit_list in assignment.  It is copied very closely
	from rsat's heap data structure.  It doesn't run any faster than the stl priority_queue
	based on the experiments that I've done. However, it has the nice property that you can
	iterate through it which is nice when you want to print it.  I will likely get rid of it
	at some point.

*/  

class Heap
{
  vector<Variable>       m_variables;
  vector<size_t>         m_position;
  size_t                 m_size;
  const vector<double>*  m_vsids_counts;

  size_t parent(size_t position) const { return position / 2; }
  size_t left_child(size_t position) const { return (position * 2) + 1; }
  size_t right_child(size_t position) const { return position * 2; }
  bool bigger(Variable v1, Variable v2) const;
  void percolate_up(size_t position);
  void percolate_down(size_t position);

public:
  Heap() : m_size(0), m_vsids_counts(NULL) { }
  Heap(size_t sz, const vector<double>* vc);
  
  bool empty() const { return m_size == 0; }
  size_t size() const { return m_size; }
  bool contains(Variable v) const { return m_position[v] != 0; }
  Variable top() const { return m_variables[1]; }
  void pop();
  void insert(Variable v);
  void clear();
  void update(Variable v) { if (contains(v)) percolate_up(v); }

  Variable operator[](size_t i) const { return m_variables[i+1]; }
  friend ostream& operator<<(ostream& os, const Heap& h);
};


////////////////////////////////////////   INLINES   ////////////////////////////////////////


inline Heap::Heap(size_t sz, const vector<double>* vc) : m_variables(sz+1,0), m_position(sz+1,0),
														 m_size(0), m_vsids_counts(vc) { }


inline bool Heap::bigger(Variable v1, Variable v2) const { return (*m_vsids_counts)[v1] > (*m_vsids_counts)[v2]; }


inline void Heap::insert(Variable v)
{
  if (contains(v)) return;
  size_t position = ++m_size;
  m_position[v] = position;
  m_variables[position] = v;
  percolate_up(position);
}


inline void Heap::pop()
{
  Variable max_element = m_variables[1];
  m_variables[1] = m_variables[m_size];  // copy the last element to the top overwriting the max element
  m_position[m_variables[1]] = 1;
  --m_size;
  m_position[max_element] = 0;  // 0 means not in the heap
  if (m_size > 1) percolate_down(1);
}

inline void Heap::clear()
{
  for (size_t i=0; i <= m_variables.size(); i++) {
	m_position[i] = 0;
	m_variables[i] = 0;
  }
}

inline void Heap::percolate_up(size_t position)
{
  Variable var = m_variables[position]; // the var we are moving up
  size_t current_position = position;
  
  size_t parent_position = parent(position);
  
  while (parent_position != 0 && bigger(var,m_variables[parent_position])) {
	Variable parent_var = m_variables[parent_position];
	m_variables[current_position] = parent_var;  // move the parent variable down
	m_position[parent_var] = current_position;
	current_position = parent_position;        
	parent_position = parent(parent_position);  // now try the next parent up the tree
  }

  m_variables[current_position] = var;
  m_position[var] = current_position;
}


inline void Heap::percolate_down(size_t position)
{
  Variable var = m_variables[position];
  size_t current_position = position;

  while (left_child(current_position) <= size()) {
	size_t l_child = left_child(current_position);
	size_t r_child = right_child(current_position);
	size_t biggest_child;
	if (r_child <= size() && bigger(m_variables[r_child],m_variables[l_child]))
	  biggest_child = r_child;
	else biggest_child = l_child;

	if (!bigger(m_variables[biggest_child],var)) break;

	m_variables[current_position] = m_variables[biggest_child]; // copy the bigger child up
	m_position[m_variables[biggest_child]] = current_position;
	current_position = biggest_child;
  }

  m_variables[current_position] = var;
  m_position[var] = current_position;
}

inline ostream& operator << (ostream& os, const Heap& h)
{
  for (size_t i=1; i <= h.size(); i++) {

    if (global_vars.output_variable_names) {
      os << "(" << global_vars.atom_name_map.lookup(h.m_variables[i]) << "," << (*h.m_vsids_counts)[h.m_variables[i]] << ")  " << flush;
    }
    else {
      os << "(" << h.m_variables[i] << "," << (*h.m_vsids_counts)[h.m_variables[i]] << ")  " << flush;
    }
  }
  return os;
}


} // end namespace zap
#endif

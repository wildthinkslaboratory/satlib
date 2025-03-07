/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#ifndef _ATOM_VALUE_MAP_H
#define _ATOM_VALUE_MAP_H

#include <iostream>
#include <vector>
#include "utilities.h"
#include "Asserts.h"
#include "Literal.h"



/***************************************************************/
/*

   CLASS: AtomValueMap

   PURPOSE: To represent and manage a set of literals or a set of
   weighted literals (as in a pseudo-Boolean constraint).  To
   determine if the set contains a given variable (atom) in constant time.

   IMPLEMENTATION: The implementation uses three vectors of int
   and a size_t m_size.  m_size represents the size of the set.
   The vector m_atoms is a list of atom ids.  Atom ids can range from
   0 to capacity (defined by the constructor or the initialize
   function) and they provide an index key for looking up the
   associated value for that atom (hence the name AtomValueMap).
   The values are stored in the vector m_value.  An atom together
   with its associated value makes up a literal or weighted literal.
   A negative value indicates a negated literal, while a positive
   value indicates a positive literal.  The absolute value of the
   value gives the weight of the literal.  The function contains(int atom)
   is done by a simple lookup of the value array.  A value of 0 means
   the atom is not present in the set.  This works since 0 is not a valid
   weight for a literal.  A third vector m_position provides a 
   reverse lookup.  If we know a given
   literal is in the AtomValueMap from checking its value entry,
   we can lookup its position in the atoms vector in constant time. 

   IMPROVEMENTS: I'd like to hide the underlying implementation better.
   Ideally, addAtom would take a Literal or PBLiteral and the user wouldn't
   need to know that negated variables are represented by negative
   values. I'm not sure if this would be more awkward always having to
   package up a literal before adding it. I'm also considering getting
   rid of the m_position vector since I so rarely use it.

   
********************************************************************/
		 

class AtomValueMap
{
protected:
  std::size_t m_size;
  std::vector<int> m_value;
  std::vector<int> m_position;
  std::vector<int> m_atoms;

 public:                   
  
  inline AtomValueMap() : m_size(0) {}
  inline AtomValueMap(std::size_t capacity);
  
  /************************* READ ****************************/    
  
  std::size_t size() const { return m_size; }
  std::size_t capacity() const { return m_value.size(); }
  bool empty() const { return m_size == 0; }

  // access by atom
  inline int getValue(std::size_t) const;
  inline int getPosition(std::size_t) const;
  inline bool contains(std::size_t) const;
  
  // access by index
  inline int getAtom(std::size_t) const;
  inline bool getSign(std::size_t) const;
  inline Literal getLiteral(std::size_t) const;
  inline int getWeight(std::size_t) const;
  
  inline void print(std::ostream& os = std::cout) const;  
  
  /********************** WRITE ******************************/          

  inline void initialize(std::size_t capacity);
  inline void reset();
  inline void clear();
  inline void addAtom(std::size_t a, int val) ;
  inline int addIfAbsent(std::size_t a, int val) ;
  // remove atom a
  inline void remove(std::size_t a) ;
  // remove the ith atom
  inline int removeByIndex(std::size_t i) ;
  inline int increment(std::size_t a, int delta) ;
  inline void insert(std::size_t a, int val, size_t position);
};


/*************************** INLINES *************************************/


inline int AtomValueMap::getValue(std::size_t i) const
{
  ASSERT(i < m_value.size());
  return m_value[i];
}


inline int AtomValueMap::getPosition(std::size_t i) const
{
  ASSERT(i < m_position.size());
  return m_position[i];
}

inline int AtomValueMap::getAtom(std::size_t i) const
{
  ASSERT(i < m_size);
  return m_atoms[i];
}

inline bool AtomValueMap::getSign(std::size_t i) const
{
  ASSERT(i < m_size);
  return m_value[m_atoms[i]] > 0 ? true : false;
}

inline Literal AtomValueMap::getLiteral(std::size_t i) const
{
  return Literal(getAtom(i),getSign(i));
}

inline int AtomValueMap::getWeight(std::size_t i) const
{
  ASSERT(i < m_size);
  return abs(m_value[m_atoms[i]]);
}

inline bool AtomValueMap::contains(std::size_t i) const
{
  ASSERT(i < m_value.size());
  return m_value[i] != 0;
}


inline AtomValueMap::AtomValueMap(std::size_t capacity):
  m_size(0),
  m_value(std::vector<int>(capacity,0)),
  m_position(std::vector<int>(capacity,0)),
  m_atoms(std::vector<int>(capacity,0)) {}



inline void AtomValueMap::initialize(std::size_t capacity)
{
  m_value.insert(m_value.end(),capacity,0);
  m_position.insert(m_position.end(),capacity,0);
  m_atoms.insert(m_atoms.end(),capacity,0);
}

inline void AtomValueMap::reset()
{
  fill(m_value.begin(),m_value.end(),0);
  m_size = 0;
}



/// similar to reset, but faster???
inline void AtomValueMap::clear()
{  
  for (std::size_t i = 0 ; i < m_size ; ++i) m_value[m_atoms[i]] = 0;
  m_size = 0; 
}


inline void AtomValueMap::addAtom(std::size_t a, int val)
{
  ASSERT(val != 0);
  ASSERT(a < m_value.size());
  m_value[a] = val;
  m_position[a] = m_size;
  m_atoms[m_size] = a;
  ++m_size;
}



/** If atom a has a value already, just return
    it.  If it's value is zero, add atom a with
    value val, and return 0. */
inline int AtomValueMap::addIfAbsent(std::size_t a, int val)
{
  ASSERT(val != 0);
  ASSERT(a < m_value.size());
  if (m_value[a]) return m_value[a];
  addAtom(a,val);
  return 0;
}


inline void AtomValueMap::remove(std::size_t a)
{
  ASSERT(a < m_value.size());
  m_atoms[m_position[a]] = m_atoms[--m_size];
  m_position[m_atoms[m_size]] = m_position[a];
  m_value[a] = 0;
}


inline int AtomValueMap::removeByIndex(std::size_t i) 
{
  ASSERT(i < m_size);
  m_value[m_atoms[i]] = 0;
  m_position[m_atoms[m_size-1]] = i;
  int a = m_atoms[i];
  m_atoms[i] = m_atoms[--m_size];
  return a;
}



inline int AtomValueMap::increment(std::size_t a, int delta)
{
  ASSERT(a < m_value.size());
  if (m_value[a]) m_value[a] += delta;
  else addAtom(a,delta);
  return m_value[a];
}

inline void AtomValueMap::insert(std::size_t a, int val, size_t position)
{
  ASSERT(val != 0);
  ASSERT(a < m_value.size());
  ASSERT(position <= m_size);
  
  for (size_t i = m_size; i > position; --i)
    {
      int atom = m_atoms[i - 1];
      m_atoms[i] = atom;
      m_position[atom]++;
    }

  m_value[a] = val;
  m_position[a] = position;
  m_atoms[position] = a;
  ++m_size;
}


inline void AtomValueMap::print(std::ostream& os) const
{
  for (std::size_t i = 0 ; i < m_size ; ++i) 
    os << m_atoms[i] << ":" << m_value[m_atoms[i]] << " ";
}

inline std::ostream& operator<<(std::ostream& os, const AtomValueMap& avm)
{
  avm.print(os);
  return os;
}

#endif




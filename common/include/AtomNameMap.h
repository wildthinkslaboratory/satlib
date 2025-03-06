#ifndef _ATOM_NAME_MAP_H
#define _ATOM_NAME_MAP_H

/***  Please see copyright notice in common.h  ***/

#include <vector>
#include <map>
#include <iostream>
#include <string>

using namespace std;

namespace zap
{

/* An AtomNameMap is just a way to associate variable names (i.e.,
strings) to ints.  We use a map in one direction and a vector in the
other.  */

class AtomNameMap : public vector<string> {
  map<string,size_t> m_atoms;

public:
  /****************************** READ ******************************/

  string lookup(unsigned i)
    { return (i > size()) ? string("unknown") : operator[](i - 1); }
  
  /****************************** WRITE ******************************/

  size_t lookup(const string & key)
    {
      map<string,size_t>::iterator p = m_atoms.find(key);
      if (p != m_atoms.end()) return p->second;
      else {
	push_back(key);
	return m_atoms[key] = size();
      }
    }
  
  void clear() { 
    m_atoms.clear(); 
    vector<string>::clear(); 
  }
  
  /****************************** PRINT ******************************/

  friend ostream& operator << (ostream& os, const AtomNameMap & anm) {
    map<string,size_t>::const_iterator p;
    for (p = anm.m_atoms.begin(); p != anm.m_atoms.end(); p++) 
      os << p->first << "   " << p->second << endl;
    return os;
  }
};


} // end namespace zap

#endif

/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/

#ifndef _ATOM_NAME_MAP_LOCAL_H
#define _ATOM_NAME_MAP_LOCAL_H
#include <map>
#include <iostream>
#include <string>



/*****************************************************************/
/*

   CLASS: AtomNameMapLocal

   PURPOSE: This is a simple class that maps variable names to
   their ID numbers.

   IMPLEMENTATION: map

********************************************************************/



class AtomNameMapLocal {
  int m_count;
  std::map<std::string,int> m_atoms;
  std::map<int,std::string> m_names;

public:

  AtomNameMapLocal() : m_count(0) {}

  /********************* READ ***********************/

  int size() const { return m_atoms.size(); }
  std::string lookup(int i)
    {
      std::map<int,std::string>::iterator p;
      p = m_names.find(i);
      if (p != m_names.end())
	return p->second;
      return std::string("unknown") ;
    }
  
  /********************* WRITE **********************/

  int lookup(std::string key)
    {
      std::map<std::string,int>::iterator p;
      p = m_atoms.find(key);
      if (p != m_atoms.end())
	return p->second;
      else
	{
	  m_count++;
	  m_atoms.insert(std::pair<std::string,int>(key,m_count));
	  m_names.insert(std::pair<int,std::string>(m_count,key));
	  return m_count;
	}
    }
  
  void clear() { m_atoms.clear(); m_names.clear(); m_count = 0; }
  
  void print(std::ostream& os = std::cout)
    {
      std::map<std::string,int>::iterator p;
      for (p = m_atoms.begin(); p != m_atoms.end(); p++) 
	os << p->first << "   " << p->second << std::endl;
    }
};

inline std::ostream& operator<<(std::ostream& os, AtomNameMapLocal& anm) {
  anm.print(os);
  return os;
}

#endif

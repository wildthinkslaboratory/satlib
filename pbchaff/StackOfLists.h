/**********************************/
/*  written by Heidi Dixon
    University of Oregon
    dixon@cirl.uoregon.edu
***********************************/


#ifndef _STACK_OF_LISTS_H
#define _STACK_OF_LISTS_H
#include <iostream>
#include <vector>

/*****************************************************************/
/*

   CLASS: StackOfLists

   PURPOSE: This is a strange sort of list with the following
   properties:

           1. The stack may contain lists that are empty lists.

	   2. Elements can only be added to the top list on the
	      stack.

	   3. The union of all the lists can be walked as a single
	      list.

	   4. The only way to remove elements is to pop the top
	      list removing all of its elements at once.

   IMPLEMENTATION: The lists elements are stored sequentially in a single
   vector of ints called m_elements, with the second list following the
   first etc..  The vector m_stack is a set of pointers into the list,
   each demarking the end of the previous list and the beginning of the next.
   

********************************************************************/



class StackOfLists {
  
 protected:
  std::vector<int> m_elements;
  std::vector<std::size_t> m_stack;

 public:

  StackOfLists() {m_stack.push_back(0);}

  /********************* READ ***********************/    

  std::size_t size() const { return m_elements.size(); }
  int elementAt(int i) const { return m_elements[i]; }
  void print(std::ostream& os = std::cout) const
    {
      for (std::size_t i=0; i < m_stack.size(); ++i)
	{
	  os << "level " << i << ": ";
	  std::size_t s,j = m_stack[i];
	  if (i == m_stack.size() - 1) s = m_elements.size();
	  else s = m_stack[i+1];
	  for ( ; j < s ; ++j)
	    os << m_elements[j] << " ";
	  os << std::endl;
	}
    }
   
  /********************* WRITE ***********************/


  void addElement(int i) { m_elements.push_back(i); }
  void addNewList() { m_stack.push_back(m_elements.size()); }
  void popTopList()
    {
      std::size_t numberPops = m_elements.size() - m_stack[m_stack.size() - 1];
      for (std::size_t i=0; i < numberPops; i++) m_elements.pop_back();
      m_stack.pop_back();
    }
 

};


inline std::ostream& operator<<(std::ostream& os, const StackOfLists& sol)
{
  sol.print(os);
  return os;
}

#endif

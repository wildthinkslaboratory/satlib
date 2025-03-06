#ifndef __POINTER__
#define __POINTER__

#include <iostream>
#include <map>
using namespace std;

namespace zap
{
/* Smart pointer class, needed for manipulating permutations and
shared vectors thereof.

Most of this is pretty straightforward.  For initialization, we can
accept a member of the class, which is passed along to the smart
pointer.  We can also initialize with an integer (typically a vector
size).  An empty initializer does not try to initialize the class
element.

For access, we have the usual copy/assign/destroy stuff, manipulating
the count in the usual way.  Equality checks for pointer equality of
the two arguments, but if either argument is actually of the value
type, then overloading converts everything to that type and uses
equality there.  We also have the usual access to * or -> together
with casts to type T, T* (take the pointer) and bool (check if the
pointer is nonzero).  Comparisons compare addresses.  */

template <class T>
class Ptr
{
 private:
  T   * m_ptr;			/* pointer to value */
  int * m_ct;			/* number of references */

 public:
  //  explicit Ptr(int n)       : m_ptr(new T(n)), m_ct(new int(1)) { }   // this one I don't get
  explicit Ptr(const T & t) : m_ptr(new T(t)), m_ct(new int(1)) { }  // this creates a copy 
  explicit Ptr(T * t)       : m_ptr(t),        m_ct(new int(1)) { }
  explicit Ptr()            : m_ptr(0),        m_ct(new int(1)) { }
  Ptr (const Ptr<T> & p)    : m_ptr(p.m_ptr),  m_ct(p.m_ct) { inc_ct(); } 
				/* copy */
  ~Ptr() { junk(); }

  Ptr<T>& operator= (const Ptr<T>& p) {
    if (this != &p) {
      junk();
      m_ptr = p.m_ptr;
      m_ct = p.m_ct;
      inc_ct();
    }
    return *this;
  }

  bool operator== (const Ptr<T>& p) const { return m_ptr == p.m_ptr; }
  //  bool operator== (const T p)       const { return *m_ptr == p; }
  bool operator!= (const Ptr<T>& p) const { return m_ptr != p.m_ptr; }
  //  bool operator!= (const T p)       const { return *m_ptr != p; }

  bool operator<  (const Ptr<T> &p) const { return m_ptr < p.m_ptr; }

  operator T * ()  const { return m_ptr; }
  operator T & ()  const { return *m_ptr; }
  operator bool () const { return m_ptr != 0; }

  T & operator* () const { return *m_ptr; }
  T * operator->() const { return m_ptr; }

 private:
  void inc_ct() { ++*m_ct; }
  void junk() { if (--*m_ct == 0) { delete m_ct; delete m_ptr; } }

  /****************************** PRINT ******************************/

  friend ostream& operator << (ostream& os, const Ptr<T> p) {
    os << *p.m_ptr;
    return os;
  }

 public:
  void dump(ostream& os = cout) const {
    os << "ptr " << m_ptr;
    if (m_ptr) { os << " ctr " << m_ct; if (m_ct) os << " val " << *m_ct; }
  }
};

} // end namespace zap
#endif

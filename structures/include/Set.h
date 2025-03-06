#ifndef _HEIDI_SET_H
#define _HEIDI_SET_H
#include <set>
using namespace std;

namespace zap
{

/* Just a little wrapper class for a set with some
   functions that I use a lot and that clutter up the
   code when you have to write them out.
*/



template <class T>
class Set : public set<T> {

public:
  T pop()
  {          // this doesn't check if the set is empty! Beware!
    T value = *(this->begin());
    set<T>::erase(this->begin());
    return value;
  }

  bool contains(T& value) { return this->find(value) != this->end(); }
  bool contains(const T& value) { return this->find(value) != this->end(); }

  void remove(T& value)
  {
    //    if (this->find(value) == this->end()) return;
    set<T>::erase(value);
  }


};

} // end namespace zap

#endif

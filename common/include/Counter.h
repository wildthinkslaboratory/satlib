#ifndef _COUNTER_H_
#define _COUNTER_H_
#include <vector>
#include <iostream>
#include "common.h"

using namespace std;

namespace zap
{

class Counter : public vector<size_t>
{
protected:
  size_t   base;
public:
  Counter(size_t number_digits, size_t b) : vector<size_t>(number_digits,0), base(b) { }
  bool operator++();
  void reset() { for (size_t i=0; i < size(); i++) operator[](i) = 0; }
  friend ostream& operator<<(ostream& os, const Counter& c) {
	for (size_t i=0; i < c.size(); i++) os << c[i];
	return os;
  }
};

class PermutationCounter : public Counter
{
public:
  PermutationCounter(size_t nd, size_t b) : Counter(nd,b), in_use(b,false) { reset(); }
  void reset();
  bool operator++();

protected:
  vector<bool>  in_use;
  bool increment(size_t index);
  bool lowest_available(size_t index);
};

class ComboCounter : public Counter
{
  protected:
  bool increment(size_t index); 
  public:
  ComboCounter(size_t number_digits, size_t max_value);
  bool operator++();
};

class PermCounterWithFixing : public PermutationCounter
{
public:
  PermCounterWithFixing(size_t nd, size_t b) : PermutationCounter(nd,b), fixed(nd,false) { }
  void fix(size_t index) { fixed[index] = true; }
  void unfix(size_t index) { fixed[index] = false; }
  void fix(size_t index, size_t value);
  const vector<size_t>& counts() const { return *this; }
  bool set_value(size_t position, size_t value);
  void reset();
  void soft_reset();
  bool can_set_value(size_t position, size_t value) const;
  bool can_set_value2(size_t position, size_t value) const;
  bool operator++();
protected:
  vector<bool> fixed;
  bool increment(size_t index);
  bool lowest_available(size_t index);
  bool random_available(size_t index);
};





  ////////////////////////////////  INLINES   ///////////////////////////////////////////////
  
  inline bool Counter::operator++()
  {
	for (size_t i=0; i < size(); i++) {
	  operator[](i)++;
	  if (operator[](i) < base) return true;
	  else operator[](i) = 0;
	}
	return false;
  }
  
  inline void PermutationCounter::reset()
  {
	for (size_t i=0; i < size(); i++)  operator[](i) = i;
	for (size_t i=0; i < base; i++) in_use[i] = (i < size() ? true : false);
  }
  
  
  inline bool PermutationCounter::increment(size_t index)
  {
	size_t i = operator[](index) + 1;               // try to find an available value that is bigger
	while (i < in_use.size() && in_use[i]) i++;
	if (i == in_use.size()) return false;
	
	in_use[operator[](index)] = false;   // mark the old value available
	in_use[i] = true;                    // the new value unavailable
	operator[](index) = i;               // assign the new value
	return true;
  }
  


  inline bool PermutationCounter::lowest_available(size_t index)
  {
	size_t i = 0;
	while (i < in_use.size() && in_use[i]) i++;
	if (i == in_use.size()) return false;
	
	in_use[i] = true;
	operator[](index) = i;
	return true;
  }
  
  
  inline bool PermutationCounter::operator++()
  {
	size_t index = size() - 1;
	while (!increment(index)) {
	  if (index == 0) return false;
	  in_use[operator[](index)] = false;
	  --index;
	}
  for (size_t i=index+1; i < size(); i++) lowest_available(i);
  return true;
  }
  
inline ComboCounter::ComboCounter(size_t number_digits, size_t max_value) : Counter(number_digits,max_value)
{
  if (max_value < number_digits) quit("Bad ComboCounter");
  for (size_t i=0; i < size(); i++) operator[](i) = i;
}

inline bool ComboCounter::increment(size_t index)
{
  size_t value = operator[](index) + 1;
  if (index == size()-1) {
	if (value > base) return false;
	operator[](index) = value;
	return true;
  }
  
  if (value >= operator[](index+1)) return false;
  operator[](index) = value;
  return true;
 
}


inline bool ComboCounter::operator++()
{
  size_t index = size() - 1;
  while (!increment(index)) {
	if (index == 0) return false;
	--index;
  }

  for (size_t i=index+1; i < size(); i++)
	operator[](i) = operator[](i-1) + 1;

  return true;
}

inline bool PermCounterWithFixing::increment(size_t index)
{
  if (fixed[index]) return false;
  size_t i = operator[](index) + 1;               // try to find an available value that is bigger
  while (i < in_use.size() && in_use[i]) i++;
  if (i == in_use.size()) return false;
  
  in_use[operator[](index)] = false;   // mark the old value available
  in_use[i] = true;                    // the new value unavailable
  operator[](index) = i;               // assign the new value
  return true;
}


inline bool PermCounterWithFixing::set_value(size_t position, size_t value)
{
  if (operator[](position) == value) return true;  // already set to that value
  if (fixed[position]) return false;               // fixed to a different value
  
  if (!in_use[value]) {                            // if that value is free assign it
	in_use[operator[](position)] = false;
	in_use[value] = true;
	operator[](position) = value;
	return true;
  }
  
  // swap with the position using that value
  for (size_t i=0; i < size(); i++) {
	if (operator[](i) == value) {
	  if (fixed[i]) return false;	  
	  size_t old_value = operator[](position);
	  operator[](position) = value;
	  if (!random_available(i)) {  // try to assign a random and different value at position i
		operator[](i) = old_value;
	  }
	  else in_use[old_value] = false;
	  return true;
	}
  }
  return false;
}

inline bool PermCounterWithFixing::can_set_value(size_t position, size_t value) const
{
  if (operator[](position) == value) return true;
  if (fixed[position]) return false; 
  if (!in_use[value]) return true;
  for (size_t i=0; i < size(); i++) {
	if (operator[](i) == value) {
	  if (fixed[i]) return false;  // the value is used and fixed
	  return true;
	}
  }
  return false;
}


inline bool PermCounterWithFixing::can_set_value2(size_t position, size_t value) const
{
  if (operator[](position) == value) return true;
  if (fixed[position]) return false; 
  if (!in_use[value]) return true;
  return false;
}



inline bool PermCounterWithFixing::operator++()
{
  size_t index = size() - 1;
  while (!increment(index)) {
	if (index == 0) return false;
	if (!fixed[index]) in_use[operator[](index)] = false;
	--index;
  }
  for (size_t i=index+1; i < size(); i++) lowest_available(i);
  return true;
}

inline bool PermCounterWithFixing::lowest_available(size_t index)
{
  if (fixed[index]) return true;
  size_t i = 0;
  while (i < in_use.size() && in_use[i]) i++;
  if (i == in_use.size()) return false;
  
  in_use[i] = true;
  operator[](index) = i;
  return true;
}


inline bool PermCounterWithFixing::random_available(size_t index)
{
  if (fixed[index]) return true;
  vector<size_t> available;
  for (size_t i=0; i < in_use.size(); i++)
	if (!in_use[i]) available.push_back(i);
  if (available.empty()) return false;

  size_t i = available[rand() % available.size()];
  in_use[i] = true;
  operator[](index) = i;
  return true;
}


// this is really brittle
inline void PermCounterWithFixing::fix(size_t index, size_t value)
{
  for (size_t i=0; i < size(); i++) if (!fixed[index]) in_use[operator[](i)] = false;
  fixed[index] = true;
  operator[](index) = value;
  in_use[value] = true;
  for (size_t i=0; i < size(); i++) if (!fixed[i]) lowest_available(i);
								  
}



inline void PermCounterWithFixing::reset()
{
  for (size_t i=0; i < size(); i++) fixed[i] = false;
  PermutationCounter::reset();
}


inline void PermCounterWithFixing::soft_reset()
{
  for (size_t i=0; i < size(); i++)  {
	if (!fixed[i]) in_use[operator[](i)] = false;
  }

  for (size_t i=0; i < size(); i++) {
	if (!fixed[i]) lowest_available(i);
  } 
}

} // end namespace zap

#endif



#ifndef _MULTIARRAY_H
#define _MULTIARRAY_H
#include <vector>
using namespace std;

namespace zap 
{

template <class T>
class MultiArray {
private:
  vector<size_t>  dimensions;
  vector<size_t>  index_multipliers;
  vector<T>       data;
public:
  MultiArray() { }
  MultiArray(const vector<size_t>& d) : dimensions(d), index_multipliers(d.size(),1)
  {
	size_t sz = 1;
	for (size_t i=0; i < dimensions.size(); i++) {
	  index_multipliers[i] = sz;
	  sz *= dimensions[i];
	}
	data.insert(data.end(),sz,T());
  }

  T& at(const vector<size_t>& index)
  { // no error checking
	size_t data_index = 0;
	for (size_t i=0; i < index_multipliers.size(); i++) {
	  data_index += index_multipliers[i] * index[i];
	}
	return data[data_index];
  }

  const T& at(const vector<size_t>& index) const
  { // no error checking
	size_t data_index = 0;
	for (size_t i=0; i < index_multipliers.size(); i++) {
	  data_index += index_multipliers[i] * index[i];
	}
	return data[data_index];
  }

  size_t size(size_t i) const { return dimensions[i]; }  // size of the ith dimension
  size_t number_dimensions() const { return dimensions.size(); }
  size_t size() const { return data.size(); }
  T& operator[](const size_t& i) { return data[i]; }
  const T& operator[](const size_t& i) const { return data[i]; }
  
};

} // end namespace zap 


#endif

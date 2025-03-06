#include "Action.h"
//#include "Counter.h"

namespace zap
{


Action::Action(const vector<size_t>& n, const set<Column>& c, const GlobalProductGroup* gg) :
  parent_group(gg), indexes(n), columns(vector<set<Column> >(1,c))
{
  sort(indexes.begin(),indexes.end());
  index_vector = IndexVector(gg->size(),false);
  for (size_t i=0; i < indexes.size(); i++)
	index_vector[indexes[i]].flip();

  Column max = *columns[0].rbegin();
  column_map = vector<int>(max+1,-1);
  set<Column>::iterator it = columns[0].begin();
  for ( ; it != columns[0].end(); it++) column_map[*it] = 0;

}  

Action::Action(const IndexVector& iv, const vector<set<Column> >& c, const GlobalProductGroup* gg) :
  parent_group(gg), index_vector(iv), columns(c)
{
  for (size_t i=0; i < index_vector.size(); i++)
	if (index_vector[i]) indexes.push_back(i);

  for (size_t i=0; i < columns.size(); i++) {
	Column max = *columns[i].rbegin();
	column_map = vector<int>(max+1,-1);
	set<Column>::iterator it = columns[i].begin();
	for ( ; it != columns[i].end(); it++) column_map[*it] = i;
  }

}  
 

ostream& operator << (ostream& os, const Action& a) {
  os << "indexes " ;
  for (size_t i=0; i < a.indexes.size(); i++)
    os << a.indexes[i] << ':'
       << ( a.parent_group ? a.parent_group->operator[](a.indexes[i]).name() : " ") <<  ' ';
  os << "\t" << "columns ";
  for (size_t i=0; i < a.columns.size(); i++) {
    set<size_t>::iterator it = a.columns[i].begin();
    for ( ; it != a.columns[i].end(); it++) os << *it << ' ';
    os << "\t";
  }
  return os;
}  




bool Action::fixes(Literal l,const vector<Column>& c) const
{ 
  for (size_t i=0; i < indexes.size(); i++) {
	if (parent_group->operator[](indexes[i]).contains(l.variable())) {
      if (find(c.begin(),c.end(),parent_group->operator[](indexes[i]).column(l)) != c.end())
		return false;
    }
  }
  return true;
}


bool Action::fixes(Literal l) const
{ 
  for (size_t i=0; i < indexes.size(); i++) {
	if (parent_group->operator[](indexes[i]).contains(l.variable())) {
      return false;
    }
  }
  return true;
}



vector<Column> Action::get_columns(Literal l) const
{
  vector<Column> answer;
  for (size_t i=0; i < indexes.size(); i++) {
	if (parent_group->operator[](indexes[i]).contains(l.variable()))
	  answer.push_back(parent_group->operator[](indexes[i]).column(l));
  }
  return answer;
}



// it's unclear what to return when a literal has a column value in the FullSym that
// is not included in m_columns 
vector<Column> Action::get_columns(const vector<Literal>& c) const
{
  set<Column> rep;
  for (size_t i=0; i < c.size(); i++) {
    vector<Column> cols = get_columns(c[i]);
    rep.insert(cols.begin(),cols.end());
  }
  return vector<Column>(rep.begin(),rep.end());
}






vector<Literal> Action::orbit(Literal point) const
{
  set<Column> cols =  get_set_columns(point);
  if (cols.size() == 0) return vector<Literal>(1,point);
  
  vector<Literal> answer;
  vector<Column> c(columns[0].begin(),columns[0].end()); // 8%
  
  size_t num_indexes = 0;
  size_t last_index = 0;
  for (size_t i=0; i < indexes.size(); i++) {
    if (parent_group->operator[](indexes[i]).contains(point.variable())) {
      num_indexes++;
      last_index = indexes[i];
    }
  }
  
  if (num_indexes <= 1) {
    return (*parent_group)[last_index].orbit(point,columns[0]); // 35 %

  }
  else {
    PfsCounter counter(cols.size(), columns[0].size(),&c);
    Literal im = point;
    set<Literal> set_answer;
    while(true) {
      set_answer.insert(im);
      if (!counter.next()) break;
      for (size_t i=0; i < indexes.size(); i++) 
		im = (*parent_group)[indexes[i]].image(im,counter.before(),counter.after());
    }
    answer.insert(answer.end(),set_answer.begin(),set_answer.end());
  }
  return answer;
}


bool Action::stabilizes_set(set<Literal>& s) const
{
  //  cout << "stabilize " << endl << *this << endl;
  set<Literal>::iterator it = s.begin();
  for ( ; it != s.end(); it++) {
	vector<Literal> o = orbit(*it);
	for (size_t i=0; i < o.size(); i++) {
	  if (s.find(o[i]) == s.end()) {
		return false;
	  }
	}
  }
  return true;
}



} // end namespace zap

#ifndef _FOASSIGNER_H
#define _FOASSIGNER_H
#include <list>

namespace zap 
{
  class NodePtr {
	public:
	size_t       row;
	size_t       col;
	NodePtr() : row(0), col(0) { }
	NodePtr(size_t r,size_t c) : row(r), col(c) { }
	bool operator==(const NodePtr& np) const
	{
	  return (row == np.row) && (col == np.col);
	}
  };
  
  class GridNode {
	public:
	bool                      on;
	list<NodePtr>::iterator   list_position;
	GridNode() { }
	GridNode(bool o, list<NodePtr>::iterator it) : on(o), list_position(it) { }
  };
  
class FOAssigner
{
  public:
  list<NodePtr>                 node_list;
  vector<vector<GridNode> >     grid;
  vector<size_t>                row_sums;
  vector<size_t>                col_sums;

  FOAssigner(size_t nr, size_t nc) : grid(nr,vector<GridNode>(nc,GridNode(false,node_list.end()))), row_sums(nr,0), col_sums(nc,0) { }

  bool empty() { return node_list.empty(); }
  size_t num_rows() const { return row_sums.size(); }
  size_t num_cols() const { return col_sums.size(); }

  void add_assignment(size_t var, size_t val)
  {
	node_list.push_front(NodePtr(var,val));
	grid[var][val] = GridNode(true,node_list.begin());
	++row_sums[var];
	++col_sums[val];
  }

  void remove_node_from_grid(NodePtr np)
  {
	grid[np.row][np.col] = GridNode(false,node_list.end());
	  --row_sums[np.row];
	  --col_sums[np.col];
  }
  
  void clear()
  {
	list<NodePtr>::iterator it = node_list.begin();
	for ( ; it != node_list.end(); it++) {
	  remove_node_from_grid(node_list.front());
	  node_list.pop_front();
	}
  }

  size_t get_value(NodePtr np)
  {
	return row_sums[np.row] + col_sums[np.col] -1;
  }

  
  list<NodePtr>::iterator  get_min_node()
  {
	list<NodePtr>::iterator it = node_list.begin();
	list<NodePtr>::iterator min = it;
	size_t min_value = get_value(*min);
	//	cout << "min_value " << min_value << endl;
	for ( ; it != node_list.end(); ++it) {
	  if (get_value(*it) < min_value) {
		//		cout << "min value " << get_value(*it) << " node " << it->row << ',' << it->col << endl;
		min = it;
	  }
	}
	return min;
  }

  void set_assignment(list<NodePtr>::iterator it)
  {
	NodePtr np = *it;
	remove_node_from_grid(np);  // remove it from the grid
	node_list.erase(it);        // remove it from the list

	for (size_t i=0; i < grid[np.row].size(); i++) {
	  if (grid[np.row][i].on) {
		node_list.erase(grid[np.row][i].list_position);
		remove_node_from_grid(NodePtr(np.row,i));
	  }
	}

	for (size_t i=0; i < grid.size(); i++) {
	  if (grid[i][np.col].on) {
		node_list.erase(grid[i][np.col].list_position);
		remove_node_from_grid(NodePtr(i,np.col));
	  }
	}
  }

  void set_assignment(size_t var, size_t val)
  {	
	NodePtr np(var,val);
	remove_node_from_grid(np);  // remove it from the grid
	node_list.remove(np);        // remove it from the list

	for (size_t i=0; i < grid[np.row].size(); i++) {
	  if (grid[np.row][i].on) {
		node_list.erase(grid[np.row][i].list_position);
		remove_node_from_grid(NodePtr(np.row,i));
	  }
	}

	for (size_t i=0; i < grid.size(); i++) {
	  if (grid[i][np.col].on) {
		node_list.erase(grid[i][np.col].list_position);
		remove_node_from_grid(NodePtr(i,np.col));
	  }
	}
  }
  
  friend ostream& operator<<(ostream& os, const FOAssigner& foa)
  {
	for (size_t i=0; i < foa.grid.size(); i++) {
	  for (size_t j=0; j < foa.grid[i].size(); j++) {
		os << foa.grid[i][j].on << ' ';
	  }
	  os << "| " << foa.row_sums[i] << endl;
	}
	for (size_t i=0; i < foa.col_sums.size(); i++) os << "--";
	os << endl;
	for (size_t i=0; i < foa.col_sums.size(); i++)
	  os << foa.col_sums[i] << ' ';
	os << endl << endl;

	list<NodePtr>::const_iterator it = foa.node_list.begin();
	for ( ; it != foa.node_list.end(); it++)
	  os << '[' << it->row << ',' << it->col << "], ";
	os << endl;
	return os;
  }
};


  
} // end namespace zap 

#endif

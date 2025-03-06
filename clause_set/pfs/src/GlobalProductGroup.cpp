#include "GlobalProductGroup.h"

namespace zap
{



// // Isn't this just a product perm?
// /* translate a permutation in the product group to a permutation on the
//    ground literals */
// Permutation GlobalProductGroup::ground_perm(const ProductPerm& p) const
// {
//   if (p.size() != size()) { cerr << "bad product perm" << endl; exit(INTERNAL_ERROR); }
  
//   Permutation result;
//   for (size_t i=0; i < p.size(); i++) {
//     result = result * operator[](i).ground_perm(p[i]);
//   }
//   result.set_sym(ProductPerm(p,this));
//   return result;
// }

// // Isn't this just a product perm?
// /* translate a permutation in the product group to a permutation on the
//    ground literals */
// Permutation GlobalProductGroup::ground_perm(const vector<RawPerm> &p) const
// {
//   if (p.size() != size()) { cerr << "bad product perm" << endl; exit(INTERNAL_ERROR); }
  
//   Permutation result;
//   vector<ColumnPerm2> cp;
//   for (size_t i=0; i < p.size(); i++) {
//     cp.push_back(ColumnPerm2(p[i]));
//     result = result * operator[](i).ground_perm(p[i]);
//   }
  
//   result.set_sym(ProductPerm(cp,this));
//   return result;
// }






// vector<Permutation> GlobalProductGroup::get_generators(const Action& a) const
// {
//   if (a.empty()) return vector<Permutation>();
//   vector<Literal> pts;
//   set<size_t>::const_iterator it = a.columns[0].begin();
//   set<size_t>::const_iterator end = a.columns[0].end();
//   for ( ; it != end; it++)
//     pts.push_back(literal(*it+1,true));
  
//   vector<Permutation> S_k = symmetric_group(pts);
//   vector<Permutation> ground_sym;
  
//   for (size_t j=0; j < S_k.size(); j++) {
//     vector<RawPerm> p(size(),RawPerm());
//     for (size_t k=0; k < a.indexes.size(); k++)
//       p[a.indexes[k]] = (RawPerm)(S_k[j]);
//     ground_sym.push_back(ground_perm(p));
//   }
//   return ground_sym;
// }



// vector<size_t> GlobalProductGroup::get_domain_point(const vector<size_t>& indexes, Literal l) const
// {
//   vector<size_t> answer;
//   for (size_t i=0; i < indexes.size(); i++) {
//     pair<bool,Column> c = operator[](indexes[i]).column(l);
//     if (c.first) answer.push_back(c.second+1);
//   }
//   return answer;
  
// }

} // end namespace zap

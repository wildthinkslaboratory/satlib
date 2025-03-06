#ifndef _CONVERTER_H
#define _CONVERTER_H

#include "InputTheory.h"
#include "ClauseSet.h"
#include "Cnf.h"
#include "common.h"
#include "Pfs.h"
#include "SymRes.h"

namespace zap
{

typedef vector<vector<pair<size_t,size_t> > > LiteralDomainValueMap;

////////////////////////////////////////   CONVERTER   ////////////////////////////////////////
/*
  The Converter class takes an InputTheory and then using the member function translateTo
  you can convert it to an actual ClauseSet of the type that you desire. We avoid a switch
  statement on the desired type by making dummy type classes and using double dispatch.
  I'm not entirely convinced of the value of this software engineering choice.  It looks
  very nice in the main call.  The converter takes an input and desired type and gives us
  the clauseSet we want in a single line.  It feels easier to maintain than a bunch of
  case statements.  Still might be overkill.
*/


class Converter {
public:
  virtual ~Converter() { }
  virtual ClauseSet* convert(ClauseSetType t) { return NULL; }
  virtual const Cnf& get_cnf() { return Cnf(); }
 
};


class PfsConverter : public Converter
{
  InputTheory&  m_input;
  Pfs           m_theory;
  SymRes        m_symres_conversion;
  Cnf           m_cnf_conversion;
  
  void fill_symbol_table();
  void build_global_groups();
  vector<vector<vector<string> > > build_argument_sets(InputClause& ic);
  vector<vector<size_t> > get_assignments(const vector<string>& var_set, size_t domain_size);
  vector<PfsClause> build_augmented(InputClause& ic);
  vector<PfsClause> build_augmented_from_group(InputClause& ic);
  ClauseSet* convert_to_cnf();
  ClauseSet* convert_to_pfs() { return &m_theory; }
  ClauseSet* convert_to_symres();
  
public:
  PfsConverter(InputTheory& input);
  ClauseSet* convert(ClauseSetType t);
  const Cnf& get_cnf() { return m_cnf_conversion; }
};



class CnfConverter : public Converter
{
  InputTheory& m_input;
  Cnf          m_theory;

  ClauseSet* convert_to_cnf() { return &m_theory; }
  ClauseSet* convert_to_pfs() { return NULL; }
public:
  CnfConverter(InputTheory& input) : m_input(input), m_theory(input) { }
  ClauseSet* convert(ClauseSetType t);
  const Cnf& get_cnf() { return m_theory; }
};


class ClauseSetBuilder {   
protected:
  InputTheory m_input;
  Converter *m_converter;
public:
  ClauseSetBuilder(const InputTheory& input);
  ~ClauseSetBuilder() { delete m_converter; }
  ClauseSet* convert_to(ClauseSetType t) { return m_converter->convert(t); }
  const Cnf& get_cnf() { return m_converter->get_cnf(); }
};


inline ClauseSet* PfsConverter::convert(ClauseSetType t)
{
  switch (t) {
  case CNF: return convert_to_cnf();
  case SYMRES: return convert_to_symres();
  case PFS: return convert_to_pfs();
  case GROUP_BASED: quit("can't convert PFS to GROUP_BASED yet"); break;
  case NOT_SPECIFIED: return convert_to_pfs();
  default: quit("can't convert PFS clause set");
  }
  return NULL;
}

inline ClauseSet* CnfConverter::convert(ClauseSetType t)
{
  switch (t) {
  case CNF: return convert_to_cnf();
  case PFS: return convert_to_pfs();
  case GROUP_BASED: quit("can't convert CNF to GROUP_BASED yet"); break;
  case NOT_SPECIFIED: return convert_to_cnf();
  default: quit("can't convert CNF clause set");
  }
  return NULL;
}

} // end namespace zap

#endif

#ifndef __HEIDI_SAT__
#define __HEIDI_SAT__
using namespace zap;

Outcome solve(ClauseSet& C);
Result unit_propagate(ClauseSet& C, Assignment& P);
AnnotatedLiteral select_branch(Assignment& P);

#endif

add_executable(pbchaff 
    main.cpp
    Clause.cpp 
    FastClause.cpp 
    LazyClauseSet.cpp 
    Mod2ClauseSet.cpp 
    PartialAssignment.cpp
    PBClauseSet.cpp 
    Solver.cpp 
    TimeTracker.cpp)

target_link_libraries(pbchaff PUBLIC front_end clause_set)

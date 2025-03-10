add_executable(zapsat 
    main.cpp
    solver.cpp 
    UPTestingLocal.cpp)

target_link_libraries(zapsat PUBLIC front_end clause_set)

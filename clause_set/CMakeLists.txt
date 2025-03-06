add_library(clause_set src/Converter.cpp)

add_subdirectory(cnf)
add_subdirectory(pfs)
add_subdirectory(symres)

target_include_directories(clause_set PUBLIC include)
target_link_libraries(clause_set common cnf pfs symres)

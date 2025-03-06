add_library(symres src/SymRes.cpp)

target_include_directories(symres PUBLIC include)
target_link_libraries(symres common cnf pfs)
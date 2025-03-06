add_library(cnf src/Cnf.cpp)

target_include_directories(cnf PUBLIC include)
target_link_libraries(cnf common)

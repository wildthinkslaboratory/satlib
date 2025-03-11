
# find_package(BISON)
# BISON_TARGET(grammar src/grammar.y src/grammar.cpp
#              DEFINES_FILE include/grammar.hpp)

# find_package(FLEX)
# flex_target(tokens src/tokens.l src/tokens.cpp)

add_library(front_end 
    src/UPTesting.cpp 
    src/front_end.cpp
    src/grammar.cpp
    src/tokens.cpp )


target_include_directories(front_end PUBLIC include)
target_link_libraries(front_end common clause_set)


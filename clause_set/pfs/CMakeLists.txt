add_library(pfs 
    src/Action.cpp
    src/FullSym.cpp
    src/GlobalProductGroup.cpp
    src/LocalSearch2.cpp 
    src/LocalSearch3.cpp 
    src/Pfs.cpp 
    src/ProductSubgroup.cpp 
    src/Transport.cpp)

target_include_directories(pfs PUBLIC include)
target_link_libraries(pfs common)
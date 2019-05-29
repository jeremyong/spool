add_executable(test test1.cpp test2.cpp main.cpp)

include(Spool)
spool(test)

add_library(test_lib lib1.cpp)
spool(test_lib)

target_link_libraries(test PUBLIC test_lib)

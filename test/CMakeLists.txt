add_executable(spool_test Main.cpp TU1.cpp TU2.cpp)

add_library(spool_test_lib_1 lib1/TU1.cpp)
add_library(spool_test_lib_2 lib2/TU1.cpp)
target_link_libraries(spool_test PUBLIC spool_test_lib_1 spool_test_lib_2)

include(Spool)
spool(spool_test_lib_1)
spool(spool_test_lib_2)
spool(spool_test)

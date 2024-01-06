add_executable(sequential main_sequential.cpp benchmark.hpp)

add_executable(farm main_farm.cpp benchmark.hpp)
add_executable(autonomicfarm main_autonomicfarm.cpp benchmark.hpp)

# implementation with fastflow
add_executable(ffautonomicfarm main_ff_autonomicfarm.cpp benchmark.hpp ffbenchmarkutils.hpp)
add_executable(fffarm main_ff_farm.cpp benchmark.hpp ffbenchmarkutils.hpp)
target_include_directories(ffautonomicfarm PRIVATE "${PROJECT_SOURCE_DIR}/extern/fastflow")
target_include_directories(fffarm PRIVATE "${PROJECT_SOURCE_DIR}/extern/fastflow")

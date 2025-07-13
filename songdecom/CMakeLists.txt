cmake_minimum_required(VERSION 3.0.0)

# Create the executable target for main.cpp
add_executable(songdecom main.cpp)

# Ensure it uses C++20
target_compile_features(songdecom PUBLIC cxx_std_20)

# Include directories
target_include_directories(songdecom PUBLIC
    ${PROJECT_SOURCE_DIR}/dependency
    ${PROJECT_SOURCE_DIR}/liblsdj/include/lsdj
)

# Link with liblsdj
target_link_libraries(songdecom liblsdj)

# Install the executable to the 'bin' directory
install(TARGETS songdecom DESTINATION bin)

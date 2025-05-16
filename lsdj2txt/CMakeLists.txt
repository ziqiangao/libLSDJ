cmake_minimum_required(VERSION 3.0.0)

# Create the executable target for main.cpp
add_executable(lsdj2txt main.cpp)

# Ensure it uses C++20
target_compile_features(lsdj2txt PUBLIC cxx_std_20)

# Include directories
target_include_directories(lsdj2txt PUBLIC
    ${PROJECT_SOURCE_DIR}/dependency
    ${PROJECT_SOURCE_DIR}/liblsdj/include/lsdj
)

# Link with liblsdj
target_link_libraries(lsdj2txt liblsdj)

# Install the executable to the 'bin' directory
install(TARGETS lsdj2txt DESTINATION bin)

cmake_minimum_required(VERSION 3.10)
project(SimpleHttpServer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Boost components
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Add executable
add_executable(http_server main.cpp)

# Link with Boost libraries
target_link_libraries(http_server PRIVATE
    Boost::system
    Boost::filesystem
)

# Create www directory for static files if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/www)

# Copy sample HTML files to the build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/www/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/www/)

# Print usage instructions
message(STATUS "Build completed. Run the server with: ./http_server 8080 ./www")
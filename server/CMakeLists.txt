cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_C_STANDARD 99)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Main server source files
set(CORE_SRC
    src/core/main.c
    src/core/server.c
    src/device/device.c
    src/socket/socket.c
    src/handler/handler.c
)

# Add the server binary
add_executable(server ${CORE_SRC})

# Include directories for header files
target_include_directories(server PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/device
    ${CMAKE_SOURCE_DIR}/src/socket
    ${CMAKE_SOURCE_DIR}/src/handler
)

# Link prebuilt shared libraries and system libs
target_link_libraries(server PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/led/libled.so
    ${CMAKE_SOURCE_DIR}/lib/buzzer/libbuzzer.so
    ${CMAKE_SOURCE_DIR}/lib/cds/libcds.so
    ${CMAKE_SOURCE_DIR}/lib/fnd/libfnd.so
    wiringPi
    pthread
    dl
)
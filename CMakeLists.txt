cmake_minimum_required(VERSION 3.20)
project(image_processor CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(image_processor
    main.cpp
    bmp.cpp
    filter_factory.cpp
)
target_compile_options(image_processor PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(image_processor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # include из папки filters

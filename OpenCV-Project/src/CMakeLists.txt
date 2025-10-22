cmake_minimum_required(VERSION 3.15)
project(OpenCVReader)

# Collect all cpp files
file(GLOB_RECURSE SRC_FILES "*.cpp")
# Create library
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# OpenCV
find_package(OpenCV REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS})
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(Tesseract REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Tesseract::libtesseract)
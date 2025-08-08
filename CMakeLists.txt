cmake_minimum_required(VERSION 3.16)

project(zask_media_converter VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# wxWidgets
find_package(wxWidgets COMPONENTS base core REQUIRED)
include(${wxWidgets_USE_FILE})

file(GLOB_RECURSE SRC_FILES
    CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

add_executable(media-converter
    ${SRC_FILES}
)

target_include_directories(media-converter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(media-converter PRIVATE ${wxWidgets_LIBRARIES})

# Name binary with OS and arch suffix
set(BUILD_OS "${CMAKE_SYSTEM_NAME}")
string(TOLOWER "${BUILD_OS}" BUILD_OS_LC)
set(BUILD_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
string(TOLOWER "${BUILD_ARCH}" BUILD_ARCH_LC)
set(BINARY_BASENAME "media-converter-${BUILD_OS_LC}-${BUILD_ARCH_LC}")
set_target_properties(media-converter PROPERTIES OUTPUT_NAME "${BINARY_BASENAME}")

# Platform-specific tweaks
if(WIN32)
    # Set the subsystem to Windows to hide console on Windows builds
    target_link_options(media-converter PRIVATE -mwindows)
endif()

install(TARGETS media-converter RUNTIME DESTINATION bin)

message(STATUS "Configured ${PROJECT_NAME} ${PROJECT_VERSION}")



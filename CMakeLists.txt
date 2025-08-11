cmake_minimum_required(VERSION 3.21)

project(zask LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If using vcpkg, pass -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# Try wxWidgets config package first (vcpkg on Windows), then fall back to module mode (Linux distros)
set(WX_COMPONENTS base core adv)
find_package(wxWidgets CONFIG QUIET COMPONENTS ${WX_COMPONENTS})
if(TARGET wx::core)
  set(ZASK_USE_WX_TARGETS TRUE)
else()
  unset(wxWidgets_FOUND)
  find_package(wxWidgets REQUIRED COMPONENTS core base adv)
  set(ZASK_USE_WX_TARGETS FALSE)
endif()

add_executable(zask WIN32
    src/main.cpp
    src/core/MediaConverter.cpp
    src/core/MediaConverter.h
    src/ui/MainFrame.h
)

if(ZASK_USE_WX_TARGETS)
  target_link_libraries(zask PRIVATE wx::base wx::core wx::adv)
else()
  include(${wxWidgets_USE_FILE})
  target_link_libraries(zask PRIVATE ${wxWidgets_LIBRARIES})
endif()

# Recommended Unicode defines for Windows builds
if(WIN32)
  target_compile_definitions(zask PRIVATE UNICODE _UNICODE)
  # vcpkg's VCPKG_APPLOCAL_DEPS=ON handles DLL copying automatically
  # No need for manual post-build commands
endif()



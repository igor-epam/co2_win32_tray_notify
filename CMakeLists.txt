#CMakeList.txt : Top - level CMake project file, do global configuration
#and include sub - projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("CO2Tracker")

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CXX_FLAGS_LIST
    "/Zi"
    "/MP" # Multi-processor compilation
    "/permissive-" # Conform to the C++ standard
    "/std:c++17" # Specify the C++ standard to use
    "/Zo" # Generate enhanced debugging information for optimized code in non-debug builds
    "/sdl" # Enable additional security checks
    "/WX" # Treat all warnings as errors
    "/Gm-" # Disable minimal rebuild
    "/GS" # Enable buffer security checks
    "/fp:precise" # Disable optimizations that could change the precision of floating-point calculations
    "/Zc:wchar_t" # Treat wchar_t as a built-in type
    "/Zc:forScope" # Force standard C++ behavior in for loop scope
    "/Zc:inline" # Remove unreferenced functions or data that are COMDAT or only have internal linkage
    "/Gd" # Use default calling convention
    "/nologo" # Suppress startup banner and informational messages
    "/EHsc" # Exception-handling behavior: Catch C++ exceptions only, extern "C" functions never throw an exception
    "/Zc:__cplusplus" # Enable updated __cplusplus macro. By default, Visual Studio always returns the value "199711L" for the __cplusplus preprocessor macro.
  )
  foreach(_flag ${CXX_FLAGS_LIST})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_flag}")
  endforeach()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2")
  set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS${buildType}} /nologo")
  add_definitions(-D_WIN32_WINNT=0x600)

find_program(CONAN_PROGRAM "conan")
IF (NOT CONAN_PROGRAM)
    execute_process (
        COMMAND pip install conan
    )
ENDIF()

#Setup conan
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

SET (Boost_USE_STATIC_LIBS TRUE)
SET (Boost_USE_MULTITHREADED ON)

conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)
set(CMAKE_MAP_IMPORTED_CONFIG_RELEASE "Release;RelWithDebInfo;")
set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO "RelWithDebInfo;Release;")
set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL "MinSizeRel;Release;RelWithDebInfo;")

SET (TRACKER_BOOST_COMPONENTS system date_time program_options)
FIND_PACKAGE (Boost 1.77.0 REQUIRED COMPONENTS ${TRACKER_BOOST_COMPONENTS} CONFIG)

#Include sub - projects.
list(APPEND CMAKE_MODULE_PATH 3rdParty/lib/cmake/mqtt_cpp_iface)
list(APPEND CMAKE_PREFIX_PATH 3rdParty/lib/cmake/mqtt_cpp_iface)
find_package(mqtt_cpp_iface REQUIRED CONFIG)
add_subdirectory (CO2Tracker)

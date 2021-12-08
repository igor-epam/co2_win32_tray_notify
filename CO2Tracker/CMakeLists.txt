# CMakeList.txt : CMake project for CO2Tracker, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_WIN32_EXECUTABLE ON)
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
# Add source to this project's executable.

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Resource.rc 
#    ${CMAKE_CURRENT_SOURCE_DIR}/Resource.rc.rc)

ADD_DEFINITIONS(-DUNICODE -DNOMINMAX -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    SET (CMAKE_CXX_FLAGS "/bigobj ${CMAKE_CXX_FLAGS}")
ENDIF ()

add_executable (CO2Tracker WIN32
    CO2Tracker.h
    CO2Tracker.cpp
    CredentialsManager.h
    CredentialsManager.cpp
    MqttClient.h
    MqttClient.cpp
    SettingsManager.h
    SettingsManager.cpp
    StateTranslator.h
    StateTranslator.cpp
    Strings.h
    TrayWindow.h
    TrayWindow.cpp
    WinApiException.h
    resource.h
    Resource.rc)

target_link_libraries(CO2Tracker mqtt_cpp_iface::mqtt_cpp_iface)

# TODO: Add tests and install targets if needed.

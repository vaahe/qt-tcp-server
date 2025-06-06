cmake_minimum_required(VERSION 3.16)
project(qt-tcp-server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automoc/uic/rcc
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Tell CMake where to find OpenCVConfig.cmake
set(OpenCV_DIR "C:/Users/User/Downloads/opencv/build")
find_package(OpenCV 4.8 REQUIRED)

# Find Qt modules
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Widgets LinguistTools Sql Network SerialPort
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets LinguistTools Sql Network SerialPort
)

# List all your sources here
set(PROJECT_SOURCES
    src/main.cpp

    src/windows/main_window/mainwindow.cpp
    src/windows/main_window/mainwindow.h
    src/windows/main_window/mainwindow.ui

    src/database/database-manager.cpp
    src/database/database-manager.h

    src/datatypes/user.h
    src/datatypes/result.h

    src/controllers/main-controller.cpp
    src/controllers/main-controller.h

    src/servers/base-tcp-server/base-tcp-server.cpp
    src/servers/base-tcp-server/base-tcp-server.h

    src/servers/sound-server/sound-server.h

    src/servers/commands-server/commands-server.h
    src/servers/commands-server/command-server.cpp

    src/servers/sensor-server/sensor-server.h
    src/servers/sensor-server/sensor-server.cpp

    src/servers/coordinates-server/coordinates-server.h
    src/servers/coordinates-server/coordinates-server.cpp

    src/serialport/serial-port-manager.cpp
    src/serialport/serial-port-manager.h

    src/utils/json-parser/json-parser.cpp
    src/utils/json-parser/json-parser.h
    src/utils/globals-manager/globals-manager.h

    src/widgets/results-widget/results-widget.cpp
    src/widgets/results-widget/results-widget.h
    src/widgets/debug-widget/debug-widget.h
    src/widgets/debug-widget/debug-widget.cpp

    src/calibration/camera-calibration.cpp
    src/calibration/camera-calibration.h

    src/utils/display-manager/display-manager.h
    src/utils/display-manager/display-manager.cpp

    src/clients/client.h

    src/clients/http-client/http-client.h
    src/clients/http-client/http-client.cpp

    src/processing/frame-processor.h
    src/processing/frame-processor.cpp

    resources/application.qrc
    qt-tcp-server_en_US.ts
)

# 1) Declare the executable
if (QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(qt-tcp-server MANUAL_FINALIZATION ${PROJECT_SOURCES}
        src/models/division/Division.h
        src/models/military-unit/MilitaryUnit.h
        src/models/result/Result.h
        src/models/subdivision/Subdivision.h
        src/models/user/User.h)
else()
    add_executable(qt-tcp-server
        ${PROJECT_SOURCES}
    )
endif()

# 2) Include directories (OpenCV, plus your src/)
target_include_directories(qt-tcp-server
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
)

# 3) Link against Qt and OpenCV
target_link_libraries(qt-tcp-server
    PRIVATE
        ${OpenCV_LIBS}
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::SerialPort
)

# 4) Translations
if (QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} qt-tcp-server_en_US.ts)
    qt_finalize_executable(qt-tcp-server)
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} qt-tcp-server_en_US.ts)
endif()

# 5) Bundle settings and install rules (as before)
if (${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qt-tcp-server)
endif()
set_target_properties(qt-tcp-server PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qt-tcp-server
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

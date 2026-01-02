cmake_minimum_required(VERSION 3.16)

project(DFCacheDelete LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(DFCacheDelete
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/CacheScanner.cpp
    src/CacheScanner.h
    src/FavoritesManager.cpp
    src/FavoritesManager.h
    src/resources.qrc
)

target_link_libraries(DFCacheDelete PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)

# Windows specific settings for GUI (hide console)
if(WIN32)
    set_target_properties(DFCacheDelete PROPERTIES WIN32_EXECUTABLE ON)
endif()

install(TARGETS DFCacheDelete
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

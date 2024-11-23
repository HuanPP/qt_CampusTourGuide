cmake_minimum_required(VERSION 3.16)

# 项目信息
project(CampusTourGuide VERSION 0.1 LANGUAGES CXX)

# 启用Qt的自动工具
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt库
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 设置项目的源文件
set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    mainwindow.ui
    Graph.cpp
    Graph.h
)

# 根据Qt版本创建可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CampusTourGuide
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(CampusTourGuide
        ${PROJECT_SOURCES}
    )
endif()

# 链接Qt库
target_link_libraries(CampusTourGuide PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(CampusTourGuide PRIVATE ${CMAKE_SOURCE_DIR})

# 针对macOS和Windows设置可执行文件属性
set_target_properties(CampusTourGuide PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 如果使用Qt6，添加最终配置
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CampusTourGuide)
endif()

cmake_minimum_required(VERSION 3.19)
project(ClientPlayer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(ClientPlayer
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    Core/Application/Applicationmanager.h Core/Application/Applicationmanager.cpp
    Core/Utils/Logger/Logger.h Core/Utils/Logger/Logger.cpp
    Core/Settings/ConfigManager.h Core/Settings/ConfigManager.cpp
    Core/License/licensemanager.h Core/License/licensemanager.cpp
    Core/Device/DeviceManager.h Core/Device/DeviceManager.cpp
    Core/License/Storage/LicenseStorage.h Core/License/Storage/LicenseStorage.cpp
    Core/License/UI/activationwindow.h Core/License/UI/activationwindow.cpp Core/License/UI/activationwindow.ui
)

target_link_libraries(ClientPlayer
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS ClientPlayer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ClientPlayer
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

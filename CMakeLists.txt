cmake_minimum_required(VERSION 3.19)
project(ClientPlayer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Multimedia
    MultimediaWidgets
    WebEngineWidgets
)

set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(OPENSSL_USE_STATIC_LIBS OFF)

find_package(OpenSSL REQUIRED)

qt_standard_project_setup()

qt_add_executable(ClientPlayer
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    Core/Application/ApplicationManager.h Core/Application/ApplicationManager.cpp
    Core/Utils/Logger/Logger.h Core/Utils/Logger/Logger.cpp
    Core/Settings/ConfigManager.h Core/Settings/ConfigManager.cpp
    Core/License/LicenseManager.h Core/License/LicenseManager.cpp
    Core/Device/DeviceManager.h Core/Device/DeviceManager.cpp
    Core/License/Storage/LicenseStorage.h Core/License/Storage/LicenseStorage.cpp
    Core/License/UI/ActivationWindow.h Core/License/UI/ActivationWindow.cpp Core/License/UI/ActivationWindow.ui
    Core/Models/Course.h Core/Models/Course.cpp
    Core/Managers/CourseManager.h Core/Managers/CourseManager.cpp
    Core/Models/Lesson.h Core/Models/Lesson.cpp
    Core/Managers/LessonManager.h Core/Managers/LessonManager.cpp
    Core/Managers/VideoManager.h Core/Managers/VideoManager.cpp
    Core/Player/VideoPlayer.h Core/Player/VideoPlayer.cpp
    Core/Progress/LessonProgress.h Core/Progress/LessonProgress.cpp
    Core/Progress/ProgressManager.h Core/Progress/ProgressManager.cpp
    Core/Progress/ProgressStorage.h Core/Progress/ProgressStorage.cpp
    Core/Security/ScreenCaptureManager.h Core/Security/ScreenCaptureManager.cpp
    Core/Security/Encryption/EncryptionManager.h Core/Security/Encryption/EncryptionManager.cpp
    Core/Crypto/CryptoManager.h Core/Crypto/CryptoManager.cpp
    Core/Crypto/EncryptionManager.h Core/Crypto/EncryptionManager.cpp
    Core/Crypto/KeyManager.h Core/Crypto/KeyManager.cpp
    Core/Crypto/VideoCryptoManager.h Core/Crypto/VideoCryptoManager.cpp
    Core/Crypto/EncryptedVideoDevice.h Core/Crypto/EncryptedVideoDevice.cpp
    Core/Offline/OfflineCacheManager.h Core/Offline/OfflineCacheManager.cpp
    Core/Server/ServerManager.h Core/Server/ServerManager.cpp
    Core/Server/ServerConfig.h Core/Server/ServerConfig.cpp
    Core/Server/ServerClient.h Core/Server/ServerClient.cpp
    Core/Player/YouTubePlayer.h Core/Player/YouTubePlayer.cpp

)





target_link_libraries(ClientPlayer
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        Qt::Multimedia
        Qt::MultimediaWidgets
        Qt6::WebEngineWidgets
        Crypt32


        OpenSSL::SSL
        OpenSSL::Crypto
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

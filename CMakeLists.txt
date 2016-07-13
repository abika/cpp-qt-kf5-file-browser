project(view_test)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.2.0")

find_package(ECM 1.1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(GenerateExportHeader)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)
include(CheckIncludeFiles)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

# Find Qt modules
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core    # QCommandLineParser, QStringLiteral
    Widgets # QApplication
)

# Find KDE modules
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons      # KAboutData
    I18n            # KLocalizedString
    WidgetsAddons   # KMessageBox
    #KDELibs4Support
    XmlGui
    KIO
    Parts
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(main_SRCS main.cpp)

add_executable(main_EXE ${main_SRCS})

target_link_libraries(main_EXE
    #Qt5::Core
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    KF5::XmlGui
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KIOFileWidgets
    KF5::Parts
)

install(TARGETS main_EXE ${INSTALL_TARGETS_DEFAULT_ARGS})

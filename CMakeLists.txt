
project(ktoshiba)
set(PROJECT_VERSION "5.0")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.4.0")

find_package(ECM 1.2.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMGenerateHeaders)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
	Core
	Widgets
	DBus
)

find_package(KF5 REQUIRED COMPONENTS
	CoreAddons
	WidgetsAddons
	Auth
	DBusAddons
	Solid
	KCMUtils
	I18n
	WindowSystem
	XmlGui
	Notifications
)

find_package(LibUDev REQUIRED)

find_package(Input REQUIRED)

include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/lib
	${CMAKE_CURRENT_BINARY_DIR}/lib
	${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(config-ktoshiba.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktoshiba.h)

add_definitions(${LIBUDEV_DEFINITIONS} -DHAVE_CONFIG_H=1 -D_DEFAULT_SOURCE)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")

add_subdirectory( lib )
add_subdirectory( kcm )
add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( po )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

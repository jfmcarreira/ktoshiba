
project(KToshiba)
set(PROJECT_VERSION_MAJOR "5")
set(PROJECT_VERSION_MINOR "3")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(QT_MIN_VERSION "5.3.2")

find_package(ECM 1.6.0 CONFIG REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMGenerateHeaders)
include(ECMSetupVersion)

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX KTOSHIBA
		  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/ktoshiba_version.h")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
	Core
	Widgets
	DBus
	Gui
)

find_package(KF5 REQUIRED COMPONENTS
	CoreAddons
	WidgetsAddons
	Auth
	DBusAddons
	KCMUtils
	I18n
	Notifications
	Config
	DocTools
)

find_package(LibMNL REQUIRED)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/lib
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${LIBMNL_INCLUDE_DIRS}
)

configure_file(config-ktoshiba.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktoshiba.h)

add_definitions(-DHAVE_CONFIG_H=1 -D_DEFAULT_SOURCE)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0 ${LIBMNL_CFLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")

add_subdirectory( lib )
add_subdirectory( kcm )
add_subdirectory( src )
add_subdirectory( doc )
add_subdirectory( icons )

ki18n_install( po )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(SystemSettings_SRCS
    ${KToshiba_SOURCE_DIR}/src/ktoshiba_debug.cpp
    systeminformation.cpp
    generalsettings.cpp
    hddprotection.cpp
    sleeputilities.cpp
    keyboardsettings.cpp
    devicemodel.cpp
    bootsettings.cpp
    powersave.cpp
    systemsettings.cpp
)

ki18n_wrap_ui(SystemSettings_SRCS
	sysinfo.ui
	general.ui
	hddprotect.ui
	sleeputils.ui
	keyboard.ui
	bootsettings.ui
	powersave.ui
)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ktoshibam\")

add_library(kcm_ktoshibam MODULE ${SystemSettings_SRCS})

target_link_libraries(kcm_ktoshibam
	ktoshibahardware
	KF5::KCMUtils
	KF5::I18n
	KF5::ConfigCore
	Qt5::DBus
	Qt5::Widgets
)

install(TARGETS kcm_ktoshibam DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcm_ktoshibam.desktop DESTINATION ${SERVICES_INSTALL_DIR})

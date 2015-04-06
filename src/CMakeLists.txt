include(KDE4Defaults)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_INSTALL_PREFIX}/share/apps/cmake/modules)

include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

add_subdirectory(helper)

set(KToshibaSources
    ktoshibadbusinterface.cpp
    ktoshibakeyhandler.cpp
    ktoshibanetlinkevents.cpp
    udevhelper.cpp
    helperactions.cpp
    fnactions.cpp
    ktoshiba.cpp
    main.cpp
)

set(SystemSettings_SRCS
    kcm/systemsettings.cpp
    helperactions.cpp
)

kde4_add_ui_files(SystemSettings_SRCS
	kcm/sysinfo.ui
	kcm/general.ui
	kcm/hddprotect.ui
	kcm/sleeputils.ui
	kcm/keyboard.ui
)

kde4_add_plugin(kcm_ktoshibam ${SystemSettings_SRCS})

qt4_add_dbus_interface(ktoshibadbusinterface.cpp net.sourceforge.ktoshiba.xml ktoshiba_interface)

qt4_add_dbus_adaptor(adaptor_SRCS net.sourceforge.ktoshiba.xml
    ktoshibadbusinterface.h KToshibaDBusInterface ktoshibadbusadaptor KToshibaDBusAdaptor)

qt4_add_resources(qtsourceview_RC_SRCS Icons.qrc)

kde4_add_ui_files(KToshibaSources statuswidget.ui)

kde4_add_executable(ktoshiba ${KToshibaSources} ${qtsourceview_RC_SRCS} ${adaptor_SRCS})

target_link_libraries(ktoshiba
	${KDE4_KDEUI_LIBS}
	${KDE4_SOLID_LIBS}
	${LIBUDEV_LIBRARY}
)

target_link_libraries(kcm_ktoshibam
	${KDE4_KDEUI_LIBS}
)

install(TARGETS ktoshiba ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ktoshiba.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES ${CMAKE_SOURCE_DIR}/rules/udev/99-ktoshiba-input.rules DESTINATION "/usr/lib/udev/rules.d")
# KCM
install(TARGETS kcm_ktoshibam DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm/kcm_ktoshibam.desktop DESTINATION ${SERVICES_INSTALL_DIR})

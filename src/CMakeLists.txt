
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_INSTALL_PREFIX}/share/apps/cmake/modules)

set(KToshibaSources
    ktoshibadbusinterface.cpp
    ktoshibakeyhandler.cpp
    ktoshibanetlinkevents.cpp
    fnactions.cpp
    ktoshiba.cpp
    main.cpp
)

qt5_add_dbus_interface(ktoshibadbusinterface.cpp net.sourceforge.ktoshiba.xml ktoshiba_interface)

qt5_add_dbus_adaptor(Adaptor_SRCS net.sourceforge.ktoshiba.xml
    ktoshibadbusinterface.h KToshibaDBusInterface ktoshibadbusadaptor KToshibaDBusAdaptor)

qt5_wrap_ui(KToshibaSources statuswidget.ui)

add_executable(ktoshiba ${KToshibaSources} ${IconResource_SRCS} ${Adaptor_SRCS})

target_link_libraries(ktoshiba
	ktoshibahardware
	KF5::WidgetsAddons
	KF5::I18n
	KF5::Notifications
	KF5::DBusAddons
	KF5::ConfigCore
	${LIBUDEV_LIBRARY}
)

install(TARGETS ktoshiba ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ktoshiba.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES ${CMAKE_SOURCE_DIR}/rules/udev/99-ktoshiba-input.rules DESTINATION "/usr/lib/udev/rules.d")

include(KDE4Defaults)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_INSTALL_PREFIX}/share/apps/cmake/modules)

include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

set(KToshibaSources
    ktoshibadbusinterface.cpp
    ktoshibakeyhandler.cpp
    fnactions.cpp
    ktoshiba.cpp
    main.cpp
    net.sourceforge.ktoshiba.xml
)

qt4_generate_dbus_interface(
    ktoshibadbusinterface.h
    net.sourceforge.ktoshiba.xml
    OPTIONS -M
)

qt4_add_resources(qtsourceview_RC_SRCS Icons.qrc)
qt4_add_dbus_adaptor(adaptor_SRCS net.sourceforge.ktoshiba.xml
    ktoshibadbusinterface.h KToshibaDBusInterface ktoshibadbusadaptor KToshibaDBusAdaptor)

kde4_add_ui_files(KToshibaSources statuswidget.ui)

kde4_add_executable(ktoshiba ${KToshibaSources} ${qtsourceview_RC_SRCS} ${adaptor_SRCS})

target_link_libraries(ktoshiba
	${KDE4_KDEUI_LIBS}
	${KDE4_SOLID_LIBS}
	${LIBNM09_LIBRARY}
	${LIBUDEV_LIBRARY}
)

if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SYSTEM_NAME MATCHES Linux)
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=iso9899:1999" )
endif(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SYSTEM_NAME MATCHES Linux)

install(TARGETS ktoshiba ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ktoshiba.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES ktoshiba.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES net.sourceforge.ktoshiba.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(PROGRAMS ktoshhelper DESTINATION ${BIN_INSTALL_DIR})

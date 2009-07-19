include(KDE4Defaults)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_INSTALL_PREFIX}/share/apps/cmake/modules)

find_package( LibKNotificationItem-1 REQUIRED )

include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LIBKNOTIFICATIONITEM-1_INCLUDE_DIR})

#set(libktoshiba_common_SRCS
#    ktoshiba.cpp
#)

set(KToshibaSources
    ktoshibadbusinterface.cpp
    fnactions.cpp
    ktoshiba.cpp
    main.cpp
)

#kde4_add_ui_files(libktoshiba_common_SRCS generalconfig.ui actionsconfig.ui editactiondialog.ui)
#kde4_add_kcfg_files(libktoshiba_common_SRCS ktoshibasettings.kcfgc)

#set(ktoshiba_KDEINIT_SRCS ${libktoshiba_common_SRCS} main.cpp ktoshibatray.cpp)

#kde4_add_kdeinit_executable(ktoshiba ${ktoshiba_KDEINIT_SRCS})

kde4_add_executable(ktoshiba ${KToshibaSources})

target_link_libraries(ktoshiba
	${KDE4_KDEUI_LIBS}
	${KDE4_KPARTS_LIBS}
	${LIBKNOTIFICATIONITEM-1_LIBRARIES}
)

if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SYSTEM_NAME MATCHES Linux)
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=iso9899:1999" )
endif(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SYSTEM_NAME MATCHES Linux)

# install
#install(TARGETS kdeinit_ktoshiba ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS ktoshiba         ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ktoshiba.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

#install(FILES ktoshiba.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
#install(FILES ktoshiba.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

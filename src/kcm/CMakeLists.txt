
set(SystemSettings_SRCS
    systemsettings.cpp
    ../helperactions.cpp
)

kde4_add_ui_files(SystemSettings_SRCS sysinfo.ui hddprotect.ui)
kde4_add_plugin(kcm_ktoshibam ${SystemSettings_SRCS})
target_link_libraries(kcm_ktoshibam ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_ktoshibam DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_ktoshibam.desktop DESTINATION ${SERVICES_INSTALL_DIR})

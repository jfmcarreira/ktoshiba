
set_source_files_properties(net.sourceforge.ktoshiba.ktoshhelper PROPERTIES NO_NAMESPACE TRUE)

kde4_add_executable(ktoshhelper ktoshhelper.cpp)
target_link_libraries(ktoshhelper ${KDE4_KDECORE_LIBS})
install(TARGETS ktoshhelper DESTINATION ${LIBEXEC_INSTALL_DIR})
kde4_install_auth_helper_files(ktoshhelper net.sourceforge.ktoshiba.ktoshhelper root)
kde4_install_auth_actions(net.sourceforge.ktoshiba.ktoshhelper net.sourceforge.ktoshiba.ktoshhelper.actions)

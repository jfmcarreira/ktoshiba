
set_source_files_properties(net.sourceforge.ktoshiba.ktoshhelper.xml PROPERTIES NO_NAMESPACE TRUE)

add_executable(ktoshhelper ktoshhelper.cpp)

target_link_libraries(ktoshhelper
	KF5::CoreAddons
	KF5::Auth
)

install(TARGETS ktoshhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(ktoshhelper net.sourceforge.ktoshiba.ktoshhelper root)
kauth_install_actions(net.sourceforge.ktoshiba.ktoshhelper net.sourceforge.ktoshiba.ktoshhelper.actions)

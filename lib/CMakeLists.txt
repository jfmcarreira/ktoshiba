
set(ktoshibapriv_SRCS ktoshibahardware.cpp)

add_library(ktoshibaprivate SHARED ${ktoshibapriv_SRCS})

target_link_libraries(ktoshibaprivate PUBLIC
	KF5::CoreAddons
	KF5::I18n
	KF5::Auth
)

set_target_properties(ktoshibaprivate PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)

add_subdirectory(helper)

install(TARGETS ktoshibaprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)


set(libktoshiba_SRCS
    ${KToshiba_SOURCE_DIR}/src/ktoshiba_debug.cpp
    ktoshibahardware.cpp
)

add_library(ktoshibahardware SHARED ${libktoshiba_SRCS})

target_link_libraries(ktoshibahardware PUBLIC
	KF5::CoreAddons
	KF5::Auth
)

set_target_properties(ktoshibahardware PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)

add_subdirectory(helper)

install(TARGETS ktoshibahardware ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

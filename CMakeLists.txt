project(ktoshiba)

# search for KDE
find_package(KDE4 4.3.90 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

configure_file(config-ktoshiba.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktoshiba.h)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES})

add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( po )

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")

project(ktoshiba)

# search for Qt and KDE
find_package(Qt4 REQUIRED)
find_package(KDE4 4.6.0 REQUIRED)
find_package(PkgConfig REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

set(INPUT_INCLUDE_DIR "/usr/include/linux")
find_file(INPUT_H "input.h" ${INPUT_INCLUDE_DIR})
if(EXISTS ${INPUT_H})
    message(STATUS "Found Kernel input include: ${INPUT_H}")
else()
    message(FATAL_ERROR "Kernel input include (input.h) not found...")
endif()

pkg_check_modules(PC_LIBUDEV libudev)
if(${PC_LIBUDEV_FOUND})
    find_path(LIBUDEV_INCLUDE_DIR libudev.h
	HINTS ${PC_LIBUDEV_INCLUDEDIR} ${PC_LIBUDEV_INCLUDE_DIRS}
	PATH_SUFFIXES libudev )

    find_library(LIBUDEV_LIBRARY udev
	HINTS ${PC_LIBUDEV_LIBDIR} ${PC_LIBUDEV_LIBRARY_DIRS} )

    message(STATUS "Found udev: ${LIBUDEV_LIBRARY}")

    set(LIBUDEV_DEFINITIONS ${PC_LIBUDEV_CFLAGS_OTHER})
    set(LIBUDEV_LIBRARIES ${LIBUDEV_LIBRARY})
    set(LIBUDEV_INCLUDE_DIRS ${LIBUDEV_INCLUDE_DIR})

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(libudev DEFAULT_MSG
	LIBUDEV_LIBRARY LIBUDEV_INCLUDE_DIR )

    mark_as_advanced(LIBUDEV_INCLUDE_DIR LIBUDEV_LIBRARY)
else(${PC_LIBUDEV_FOUND})
    message(FATAL_ERROR "udev could not be found...")
endif(${PC_LIBUDEV_FOUND})

configure_file(config-ktoshiba.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktoshiba.h)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${LIBUDEV_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES})

add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( po )

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")

project(ktoshiba)

# search for Qt and KDE
find_package(Qt4 REQUIRED)
find_package(KDE4 4.3.90 REQUIRED)
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

set(NM09_LIBRARY_DIR "/usr/lib")
find_library(NM09_LIB solidcontrolnm09 HINTS ${NM09_LIBRARY_DIR})
if(EXISTS ${NM09_LIB})
    message(STATUS "Found NetworkManager-KDE4 0.9.x library dir: ${NM09_LIBRARY_DIR}")

    set(NM09_INCLUDE_DIR "/usr/include/solid/controlnm09")
    find_file(NM09_H "networkmanager.h" ${NM09_INCLUDE_DIR})
    if(EXISTS ${NM09_H})
        set(LIBNM09_LIBRARY ${NM09_LIB})
        message(STATUS "Found NetworkManager-KDE4 0.9.x include dir: ${NM09_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "NetworkManager-KDE4 0.9.x include directory not found, please install devel package")
    endif()
else()
    message(FATAL_ERROR "Could not find libsolidcontrol09.so, please install NetworkManager-KDE4 0.9.x")
endif()

pkg_check_modules(PC_LIBUDEV libudev)
set(LIBUDEV_DEFINITIONS ${PC_LIBUDEV_CFLAGS_OTHER})

find_path(LIBUDEV_INCLUDE_DIR libudev.h
	HINTS ${PC_LIBUDEV_INCLUDEDIR} ${PC_LIBUDEV_INCLUDE_DIRS}
	PATH_SUFFIXES libudev )

find_library(LIBUDEV_LIBRARY udev
	HINTS ${PC_LIBUDEV_LIBDIR} ${PC_LIBUDEV_LIBRARY_DIRS} )

set(LIBUDEV_LIBRARIES ${LIBUDEV_LIBRARY})
set(LIBUDEV_INCLUDE_DIRS ${LIBUDEV_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libudev DEFAULT_MSG
	LIBUDEV_LIBRARY LIBUDEV_INCLUDE_DIR )

mark_as_advanced(LIBUDEV_INCLUDE_DIR LIBUDEV_LIBRARY)

configure_file(config-ktoshiba.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktoshiba.h)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${LIBUDEV_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES})

add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( po )

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")

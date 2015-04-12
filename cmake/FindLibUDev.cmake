#
# Copyright (C) 2015, Azael Avalos <coproscefalo@gmail.com>
#

find_package(PkgConfig REQUIRED)

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
    find_package_handle_standard_args(LIBUDEV DEFAULT_MSG
	LIBUDEV_LIBRARY LIBUDEV_INCLUDE_DIR
    )

    mark_as_advanced(LIBUDEV_INCLUDE_DIR LIBUDEV_LIBRARY)
endif(${PC_LIBUDEV_FOUND})

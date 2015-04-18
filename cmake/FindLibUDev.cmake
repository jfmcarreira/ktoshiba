#
# Copyright (C) 2015, Azael Avalos <coproscefalo@gmail.com>
#

include(FindPackageHandleStandardArgs)

find_path(LIBUDEV_INCLUDE_DIR libudev.h)

find_library(LIBUDEV_LIBRARY udev)

find_package_handle_standard_args(LIBUDEV DEFAULT_MSG
	LIBUDEV_LIBRARY
	LIBUDEV_INCLUDE_DIR
)

mark_as_advanced(LIBUDEV_INCLUDE_DIR LIBUDEV_LIBRARY)

#
# Copyright 2015, Azael Avalos <coproscefalo@gmail.com>
#

include(FindPackageHandleStandardArgs)

set(LINUX_INCLUDE_DIR "/usr/include/linux")

find_file(INPUT_H "input.h" PATHS ${LINUX_INCLUDE_DIR})
find_file(TOSHIBA_H "toshiba.h" PATHS ${LINUX_INCLUDE_DIR})

find_package_handle_standard_args(LINUXINCLUDES DEFAULT_MSG
	INPUT_H
	TOSHIBA_H
)

mark_as_advanced(INPUT_H TOSHIBA_H)
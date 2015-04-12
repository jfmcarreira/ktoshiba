#
# Copyright 2015, Azael Avalos <coproscefalo@gmail.com>
#

include(FindPackageHandleStandardArgs)

set(INPUT_INCLUDE_DIR "/usr/include/linux")

find_file(INPUT_H "input.h" PATHS ${INPUT_INCLUDE_DIR})

find_package_handle_standard_args(INPUT DEFAULT_MSG INPUT_H)

mark_as_advanced(INPUT_H)

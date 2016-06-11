#
# Copyright (C) 2015, Azael Avalos <coproscefalo@gmail.com>
#

include(FindPackageHandleStandardArgs)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBMNL REQUIRED libmnl)

mark_as_advanced(LIBMNL_INCLUDE_DIRS LIBMNL_LIBRARIES)

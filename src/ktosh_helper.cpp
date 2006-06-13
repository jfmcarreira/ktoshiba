/*
 *  ktosh_helper.cpp - KToshiba Helper
 *
 *  Copyright (c) 2004-2006 Azael Avalos <coproscefalo@gmail.com>
 *
 *  Based on acpi_helper from klaptopdaemon
 *  Copyright (c) 2002 Paul Campbell <paul@taniwha.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define USAGE \
"Usage: ktosh_helper [option]\n\n\
Where options are:\n\
\t--std           Activates Suspend To Disk.\n\
\t--str           Activates Suspend To RAM.\n\
\t--help          Displays this text.\n\
"

int main(int argc, char **argv)
{
	int fd, i;

	::close(0);	// we're setuid - this is just in case
	for (i = 1; i < argc; i++)
	if (strcmp(argv[i], "--std") == 0 || strcmp(argv[i], "--suspend-to-disk") == 0 || strcmp(argv[i], "--hibernate") == 0) {
		sync();
		sync();
		fd = open("/proc/acpi/sleep", O_RDWR);
		if (fd < 0) exit(1);
		write(fd, "4", 1);
		close(fd);
		setuid(getuid());	// drop all priority asap
		exit(0);
	} else
	if (strcmp(argv[i], "--str") == 0 || strcmp(argv[i], "--suspend-to-ram") == 0 || strcmp(argv[i], "--suspend") == 0) {
		sync();
		sync();
		fd = open("/proc/acpi/sleep", O_RDWR);
		if (fd < 0) exit(1);
		write(fd, "3", 1);
		close(fd);
		setuid(getuid());	// drop all priority asap
		exit(0);
	} else
	if (strcmp(argv[i], "--help") == 0)
		goto usage;
	else {
usage:
		setuid(getuid());	// drop all priority asap
		fprintf(stderr, USAGE, argv[0]);
		exit(1);
	}
	goto usage;
}

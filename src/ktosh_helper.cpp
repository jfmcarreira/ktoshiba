/*
 *  ktosh_helper.cpp - KToshiba Helper
 *
 *  Copyright (c) 2004 Azael Avalos <neftali@utep.edu>
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
//#include <sys/stat.h>

int main(int argc, char **argv)
{
	int i;
	int err;

	::close(0);	// we're setuid - this is just in case
	for (i = 1; i < argc; i++)
	if (strcmp(argv[i], "--enable") == 0 || strcmp(argv[i], "-enable") == 0 || strcmp(argv[i], "-e") == 0) {
		sync();
		sync();
		::setuid(::geteuid());
		::execl("/bin/sh", "-c", "/sbin/ifdown", "eth0", 0);
		::execl("/bin/sh", "-c", "/sbin/ifup", "ath0", 0);
		::execl("/bin/sh", "-c", "/sbin/ifup", "wlan0", 0);
		exit(0);
	} else 
	if (strcmp(argv[i], "--disable") == 0 || strcmp(argv[i], "-disable") == 0 || strcmp(argv[i], "-d") == 0) {
		sync();
		sync();
		::setuid(::geteuid());
		::execl("/bin/sh", "-c", "/sbin/ifdown", "ath0", 0);
		::execl("/bin/sh", "-c", "/sbin/ifdown", "wlan0", 0);
		::execl("/bin/sh", "-c", "/sbin/ifup", "eth0", 0);
		exit(0);
	}
	else {
usage:
		setuid(getuid());	// drop all priority asap
		fprintf(stderr, "Usage: %s [--enable] [--disable] [device]\n", argv[0]);
		exit(1);
	}
	goto usage;
}

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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/major.h>
#include <linux/hdreg.h>

#define USAGE \
"Usage: ktosh_helper [option]\n\n\
Where options are:\n\
\t--std           Activates Suspend To Disk.\n\
\t--str           Activates Suspend To RAM.\n\
\t--unregister    Try to unregister the device from the IDE driver.\n\
\t--rescan        Try to rescan the IDE bus to find devices.\n\
\t--help          Displays this text.\n\
"

int selectBayUnregister()
{
	struct stat buf;

	/* check the device exists */

	if (stat("/dev/hda", &buf))
		return -1;

	/* make sure this is IDE channel 0 */

	if (major(buf.st_rdev) != IDE0_MAJOR)
		return -1;

	/* open the device for sending commands */

	int fd = open("/dev/hda", O_RDONLY);
	if (fd < 0)
		return -1;

	/* request a device in the SelectBay is unregistered */

	int err = -1;
	for (int loop = 0; loop < 2; loop++) {
		err = ioctl(fd, HDIO_UNREGISTER_HWIF, 1);
		if (err == 0)
			break;
		sleep(1);
	}

	if (err == -1)
		return -1;

	close(fd);

	return err;
};

int selectBayRescan()
{
	int args[] = {0x170, 0, 15};
	struct stat buf;

	if (stat("/dev/hda", &buf))
		return -1;

	if (major(buf.st_rdev) != IDE0_MAJOR)
		return -1;

	int fd = open("/dev/hda", O_RDONLY);
	if (fd < 0)
		return -1;

	/* loop as sometimes it fails for no apparent reason */

	int err = -1;
	for (int loop = 0; loop < 2; loop++) {
		err = ioctl(fd, HDIO_SCAN_HWIF, args);
		if (err == 0)
			break;
		sleep(1);
	}

	if (err == -1)
		return -1;

	close(fd);

	return err;
};

int main(int argc, char **argv)
{
	int fd;
	int i;
	int err;

	::close(0);	// we're setuid - this is just in case
	for (i = 1; i < argc; i++)
	if (strcmp(argv[i], "--std") == 0 || strcmp(argv[i], "--suspend-to-disk") == 0) {
		sync();
		sync();
		fd = open("/proc/acpi/sleep", O_RDWR);
		if (fd < 0) exit(1);
		write(fd, "4", 1);
		close(fd);
		setuid(getuid());	// drop all priority asap
		exit(0);
	} else
	if (strcmp(argv[i], "--str") == 0 || strcmp(argv[i], "--suspend-to-ram") == 0) {
		sync();
		sync();
		fd = open("/proc/acpi/sleep", O_RDWR);
		if (fd < 0) exit(1);
		write(fd, "3", 1);
		close(fd);
		setuid(getuid());	// drop all priority asap
		exit(0);
	} else
	if (strcmp(argv[i], "--unregister") == 0 || strcmp(argv[i], "-unregister") == 0 || strcmp(argv[i], "-u") == 0) {
		err = selectBayUnregister();
		exit(err);
	} else
	if (strcmp(argv[i], "--rescan") == 0 || strcmp(argv[i], "-rescan") == 0 || strcmp(argv[i], "-r") == 0) {
		err = selectBayRescan();
		exit(err);
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

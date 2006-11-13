/* hci.c -- Hardware Configuration Interface
 *
 * Copyright (c) 1998-2000  Jonathan A. Buzzard (jonathan@buzzard.org.uk)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "hci.h"


int HciFunction(SMMRegisters *regs)
{
	int fd;

	if ((fd = open(TOSH_DEVICE, O_RDWR)) < 0)
		return HCI_FAILURE;

	if (access(TOSH_PROC, R_OK)) {
		close(fd);
		return HCI_FAILURE;
	}

	if (ioctl(fd, TOSH_SMM, regs) < 0) {
		close(fd);
		return (int) (regs->eax & 0xff00)>>8;
	}
	close(fd);

	return (int) (regs->eax & 0xff00)>>8;
}


/*
 * Return the BIOS version of the laptop
 */
int HciGetBiosVersion(void)
{
	FILE *str;
	int major,minor;
	char buffer[64];

	if (access(TOSH_PROC, R_OK))
		return -1;

	/* open /proc/toshiba for reading */
	if (!(str = fopen(TOSH_PROC, "r")))
		return -1;

	/* scan in the information */
	fgets(buffer, sizeof(buffer) - 1, str);
	fclose(str);
	buffer[sizeof(buffer) - 1] = '\0';
	sscanf(buffer, "%*s %*x %*d.%*d %d.%d %*x %*x\n", &major, &minor);

	/* return the information */

	return (major * 0x100) + minor;
}


/*
 * Get the Toshiba machine identification number
 */
int HciGetMachineID(int *id)
{
	FILE *str;
	char buffer[64];

	if (access(TOSH_PROC, R_OK))
		return HCI_FAILURE;

	/* open /proc/toshiba for reading */
	if (!(str = fopen(TOSH_PROC, "r")))
		return HCI_FAILURE;

	/* scan in the information */
	fgets(buffer, sizeof(buffer) - 1, str);
	fclose(str);
	buffer[sizeof(buffer) - 1] = '\0';
	sscanf(buffer, "%*s %x %*d.%*d %*d.%*d %*x %*x\n", id);

	return HCI_SUCCESS;
}


/*
 * Return the LCD Panel type
 */
int HciGetLCDPanelType(int *resolution, int *type)
{
	SMMRegisters regs;

	regs.eax = HCI_GET;
	regs.ebx = HCI_FLAT_PANEL;
	HciFunction(&regs);

	*resolution = (regs.ecx & 0xff00)>>8;	
	*type = regs.ecx & 0xff;	

	return HCI_SUCCESS;
}

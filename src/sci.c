/* sci.c -- System Configuration Interface
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

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/ioctl.h>

#include"sci.h"


/*
 * Is this a supported Machine? (ie. is it a Toshiba)
 */
int SciSupportCheck(int *version)
{
	SMMRegisters regs;
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR))<0)
		return SCI_FAILURE;

	if (access(TOSH_PROC, R_OK)) {
		close(fd);
		return SCI_FAILURE;
	}

	regs.eax = 0xf0f0;
	regs.ebx = 0x0000;
	regs.ecx = 0x0000;
	regs.edx = 0x0000;

	if (ioctl(fd, TOSH_SMM, &regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	*version = (int) regs.edx;

	return (int) (regs.eax & 0xff00)>>8;
}


/*
 * Open an interface to the Toshiba hardware.
 *
 *   Note: Set and Get will not work unless an interface has been opened.
 */
int SciOpenInterface(void)
{
	SMMRegisters regs;
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR ))<0)
		return SCI_FAILURE;

	regs.eax = 0xf1f1;
	regs.ebx = 0x0000;
	regs.ecx = 0x0000;

	if (ioctl(fd, TOSH_SMM, &regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	return (int) (regs.eax & 0xff00)>>8;
}


/*
 * Close any open interface to the hardware
 */
int SciCloseInterface(void)
{
	SMMRegisters regs;
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR ))<0)
		return SCI_FAILURE;

	regs.eax = 0xf2f2;
	regs.ebx = 0x0000;
	regs.ecx = 0x0000;

	if (ioctl(fd, TOSH_SMM, &regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	return (int) (regs.eax & 0xff00)>>8;
}


/*
 * Get the setting of a given mode of the laptop
 */
int SciGet(SMMRegisters *regs)
{
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR ))<0)
		return SCI_FAILURE;

	/*regs->eax = 0xf3f3;*/
	regs->eax = 0xf300;

	if (ioctl(fd, TOSH_SMM, regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	return (int) (regs->eax & 0xff00)>>8;
}


/*
 * Set the setting of a given mode of the laptop
 */
int SciSet(SMMRegisters *regs)
{
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR ))<0)
		return SCI_FAILURE;

	/*regs->eax = 0xf4f4;*/
	regs->eax = 0xf400;

	if (ioctl(fd, TOSH_SMM, regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	return (int) (regs->eax & 0xff00)>>8;
}


/*
 * Get the status of the AC Power on a Toshiba laptop.
 */
int SciACPower(void)
{
	SMMRegisters regs;
	int fd;

	if (access(TOSH_PROC, R_OK))
		return SCI_FAILURE;

	if ((fd=open(TOSH_DEVICE, O_RDWR))<0)
		return SCI_FAILURE;

	regs.eax = 0xfefe;
	regs.ebx = 0x0003;
	regs.ecx = 0x0000;
	regs.edx = 0x0000;

	if (ioctl(fd, TOSH_SMM, &regs)<0) {
		close(fd);
		return 0;
	}
	close(fd);

	return (int) regs.ecx;
}

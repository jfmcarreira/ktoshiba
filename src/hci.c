/* hci.c -- Hardware Configuration Interface
 *
 * Copyright (c) 1998-2000  Jonathan A. Buzzard (jonathan@buzzard.org.uk)
 *   
 * $Log$
 * Revision 1.1  2004/12/06 19:43:32  coproscefalo
 * Initial revision
 *
 * Revision 1.5  2002/01/27 13:22:57  jab
 * updated list of machine ID's
 *
 * Revision 1.4  2001/10/05 13:04:43  jab
 * checked in change to using kernel module
 *
 * Revision 1.3  1999/12/12 11:33:39  jab
 * changed assembler to save registers, should make the programs stabler
 * slightly fudged addition to GetMachineID to get SCTTable ID's
 *
 * Revision 1.2  1999/08/15 10:43:28  jab
 * removed the HciGet and HciSet and replaced with HciFunction
 *
 * Revision 1.1  1999/03/11 20:27:06  jab
 * Initial revision
 *
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

static const char rcsid[]="$Id$";

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/ioctl.h>

#include"hci.h"


int HciFunction(SMMRegisters *regs)
{
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR))<0)
		return HCI_FAILURE;

	if (access(TOSH_PROC, R_OK)) {
		close(fd);
		return HCI_FAILURE;
	}

	if (ioctl(fd, TOSH_SMM, regs)<0) {
		close(fd);
		return HCI_FAILURE;
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

	fgets(buffer, sizeof(buffer)-1, str);
	fclose(str);
	buffer[sizeof(buffer)-1] = '\0';
	sscanf(buffer, "%*s %*x %*d.%*d %d.%d %*x %*x\n", &major, &minor);

	/* return the information */

	return (major*0x100)+minor;
}


/*
 * Get the Toshiba machine identification number
 *
 *   Below is a list of known ID's and the models.
 *
 *     0xfc00:  Satellite 2140CDS/2180CDT/2675DVD 
 *     0xfc01:  Satellite 2710xDVD 
 *     0xfc02:  Satellite Pro 4270CDT//4280CDT/4300CDT/4340CDT 
 *     0xfc04:  Portege 3410CT, 3440CT 
 *     0xfc08:  Satellite 2100CDS/CDT 1550CDS 
 *     0xfc09:  Satellite 2610CDT, 2650XDVD 
 *     0xfc0a:  Portage 7140 
 *     0xfc0b:  Satellite Pro 4200 
 *     0xfc0c:  Tecra 8100x 
 *     0xfc0f:  Satellite 2060CDS/CDT 
 *     0xfc10:  Satellite 2550/2590 
 *     0xfc11:  Portage 3110CT 
 *     0xfc12:  Portage 3300CT 
 *     0xfc13:  Portage 7020CT 
 *     0xfc15:  Satellite 4030/4030X/4050/4060/4070/4080/4090/4100X CDS/CDT 
 *     0xfc17:  Satellite 2520/2540 CDS/CDT 
 *     0xfc18:  Satellite 4000/4010 XCDT 
 *     0xfc19:  Satellite 4000/4010/4020 CDS/CDT 
 *     0xfc1a:  Tecra 8000x 
 *     0xfc1c:  Satellite 2510CDS/CDT 
 *     0xfc1d:  Portage 3020x 
 *     0xfc1f:  Portage 7000CT/7010CT 
 *     0xfc39:  T2200SX 
 *     0xfc40:  T4500C 
 *     0xfc41:  T4500 
 *     0xfc45:  T4400SX/SXC 
 *     0xfc51:  Satellite 2210CDT, 2770XDVD 
 *     0xfc52:  Satellite 2775DVD, Dynabook Satellite DB60P/4DA 
 *     0xfc53:  Portage 7200CT/7220CT, Satellite 4000CDT 
 *     0xfc54:  Satellite 2800DVD 
 *     0xfc56:  Portage 3480CT 
 *     0xfc57:  Satellite 2250CDT
 *     0xfc5a:  Satellite Pro 4600 
 *     0xfc5d:  Satellite 2805 
 *     0xfc5f:  T3300SL 
 *     0xfc61:  Tecra 8200 
 *     0xfc64:  Satellite 1800 
 *     0xfc69:  T1900C 
 *     0xfc6a:  T1900 
 *     0xfc6d:  T1850C 
 *     0xfc6e:  T1850 
 *     0xfc6f:  T1800 
 *     0xfc72:  Satellite 1800 
 *     0xfc7e:  T4600C 
 *     0xfc7f:  T4600 
 *     0xfc8a:  T6600C 
 *     0xfc91:  T2400CT 
 *     0xfc97:  T4800CT 
 *     0xfc99:  T4700CS 
 *     0xfc9b:  T4700CT 
 *     0xfc9d:  T1950 
 *     0xfc9e:  T3400/T3400CT 
 *     0xfcb2:  Libretto 30CT 
 *     0xfcba:  T2150 
 *     0xfcbe:  T4850CT 
 *     0xfcc0:  Satellite Pro 420x 
 *     0xfcc1:  Satellite 100x 
 *     0xfcc3:  Tecra 710x/720x 
 *     0xfcc6:  Satellite Pro 410x 
 *     0xfcca:  Satellite Pro 400x 
 *     0xfccb:  Portage 610CT 
 *     0xfccc:  Tecra 700x 
 *     0xfccf:  T4900CT 
 *     0xfcd0:  Satellite 300x 
 *     0xfcd1:  Tecra 750CDT 
 *     0xfcd2:  Vision Connect -- what is this??? 
 *     0xfcd3:  Tecra 730XCDT
 *     0xfcd4:  Tecra 510x 
 *     0xfcd5:  Satellite 200x 
 *     0xfcd7:  Satellite Pro 430x 
 *     0xfcd8:  Tecra 740x 
 *     0xfcd9:  Portage 660CDT 
 *     0xfcda:  Tecra 730CDT 
 *     0xfcdb:  Portage 620CT 
 *     0xfcdc:  Portage 650CT 
 *     0xfcdd:  Satellite 110x 
 *     0xfcdf:  Tecra 500x 
 *     0xfce0:  Tecra 780DVD 
 *     0xfce2:  Satellite 300x 
 *     0xfce3:  Satellite 310x 
 *     0xfce4:  Satellite Pro 490x 
 *     0xfce5:  Libretto 100CT 
 *     0xfce6:  Libretto 70CT 
 *     0xfce7:  Tecra 540x/550x 
 *     0xfce8:  Satellite Pro 470x/480x 
 *     0xfce9:  Tecra 750DVD 
 *     0xfcea:  Libretto 60 
 *     0xfceb:  Libretto 50CT 
 *     0xfcec:  Satellite 320x/330x, Satellite 2500CDS 
 *     0xfced:  Tecra 520x/530x 
 *     0xfcef:  Satellite 220x, Satellite Pro 440x/460x 
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

	fgets(buffer, sizeof(buffer)-1, str);
	fclose(str);
	buffer[sizeof(buffer)-1] = '\0';
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

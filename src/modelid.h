/***************************************************************************
 *   Copyright (C) 2004-2006 by Azael Avalos                               *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef MODEL_ID_H
#define MODEL_ID_H

#include <qstring.h>

#include <kmessagebox.h>
#include <klocale.h>

QString modelID(int id)
{
	QString mod;

	switch (id) {
		case -1:
			KMessageBox::queuedMessageBox(0, KMessageBox::Information,
							i18n("Could not get model id.\nWeird"));
			break;
		case 0x0db0:
			mod = "Satellite M30";
			break;
		case 0xe8e2:
			mod = "Satellite Pro M30";
			break;
		case 0xfc00:
			mod = "Satellite 2140CDS/2180CDT/2675DVD";
			break;
		case 0xfc01:
			mod = "Satellite 2710xDVD";
			break;
		case 0xfc02:
			mod = "Satellite Pro 4270CDT/4280CDT/4300CDT/4340CDT";
			break;
		case 0xfc04:
			mod = "Portege 3410CT/3440CT";
			break;
		case 0xfc08:
			mod = "Satellite 2100CDS/CDT 1550CDS";
			break;
		case 0xfc09:
			mod = "Satellite 2610CDT/2650XDVD";
			break;
		case 0xfc0a:
			mod = "Portage 7140";
			break;
		case 0xfc0b:
			mod = "Satellite Pro 4200";
			break;
		case 0xfc0c:
			mod = "Tecra 8100x";
			break;
		case 0xfc0f:
			mod = "Satellite 2060CDS/CDT";
			break;
		case 0xfc10:
			mod = "Satellite 2550/2590";
			break;
		case 0xfc11:
			mod = "Portage 3110CT";
			break;
		case 0xfc12:
			mod = "Portage 3300CT";
			break;
		case 0xfc13:
			mod = "Portage 7020CT";
			break;
		case 0xfc15:
			mod = "Satellite 4030/4030X/4050/4060/4070/4080/4090/4100XCDS/CDT";
			break;
		case 0xfc17:
			mod = "Satellite 2520/2540CDS/CDT";
			break;
		case 0xfc18:
			mod = "Satellite 4000/4010XCDT";
			break;
		case 0xfc19:
			mod = "Satellite 4000/4010/4020CDS/CDT";
			break;
		case 0xfc1a:
			mod = "Tecra 8000x";
			break;
		case 0xfc1c:
			mod = "Satellite 2510CDS/CDT";
			break;
		case 0xfc1d:
			mod = "Portage 3020x";
			break;
		case 0xfc1f:
			mod = "Portage 7000CT/7010CT";
			break;
		case 0xfc39:
			mod = "T2200SX";
			break;
		case 0xfc40:
			mod = "T4500C";
			break;
		case 0xfc41:
			mod = "T4500";
			break;
		case 0xfc45:
			mod = "T4400SX/SXC";
			break;
		case 0xfc51:
			mod = "Satellite 2210CDT/2770XDVD";
			break;
		case 0xfc52:
			mod = "Satellite 2775DVD/Dynabook Satellite DB60P/4DA";
			break;
		case 0xfc53:
			mod = "Portage 7200CT/7220CT, Satellite 4000CDT";
			break;
		case 0xfc54:
			mod = "Satellite 2800DVD";
			break;
		case 0xfc56:
			mod = "Portage 3480CT";
			break;
		case 0xfc57:
			mod = "Satellite 2250CDT";
			break;
		case 0xfc5a:
			mod = "Satellite Pro 4600";
			break;
		case 0xfc5d:
			mod = "Satellite 2805";
			break;
		case 0xfc5f:
			mod = "T3300SL";
			break;
		case 0xfc61:
			mod = "Tecra 8200";
			break;
		case 0xfc64:
			mod = "Satellite 1800";
			break;
		case 0xfc69:
			mod = "T1900C";
			break;
		case 0xfc6a:
			mod = "T1900";
			break;
		case 0xfc6d:
			mod = "T1850C";
			break;
		case 0xfc6e:
			mod = "T1850";
			break;
		case 0xfc6f:
			mod = "T1800";
			break;
		case 0xfc72:
			mod = "Satellite 1800";
			break;
		case 0xfc7a:
			mod = "Satellite 5100";
			break;
		case 0xfc7e:
			mod = "T4600C";
			break;
		case 0xfc7f:
			mod = "T4600";
			break;
		case 0xfc8a:
			mod = "T6600C";
			break;
		case 0xfc91:
			mod = "T2400CT";
			break;
		case 0xfc97:
			mod = "T4800CT";
			break;
		case 0xfc99:
			mod = "T4700CS";
			break;
		case 0xfc9b:
			mod = "T4700CT";
			break;
		case 0xfc9d:
			mod = "T1950";
			break;
		case 0xfc9e:
			mod = "T3400/T3400CT";
			break;
		case 0xfca5:
			mod = "Satellite 5001";
			break;
		case 0xfca9:
			mod = "Satellite 1410";
			break;
		case 0xfcb2:
			mod = "Libretto 30CT";
			break;
		case 0xfcba:
			mod = "T2150";
			break;
		case 0xfcbe:
			mod = "T4850CT";
			break;
		case 0xfcc0:
			mod = "Satellite Pro 420x";
			break;
		case 0xfcc1:
			mod = "Satellite 100x";
			break;
		case 0xfcc3:
			mod = "Tecra 710x/720x";
			break;
		case 0xfcc6:
			mod = "Satellite Pro 410x";
			break;
		case 0xfcca:
			mod = "Satellite Pro 400x";
			break;
		case 0xfccb:
			mod = "Portage 610CT";
			break;
		case 0xfccc:
			mod = "Tecra 700x";
			break;
		case 0xfccf:
			mod = "T4900CT";
			break;
		case 0xfcd0:
			mod = "Satellite 300x";
			break;
		case 0xfcd1:
			mod = "Tecra 750CDT";
			break;
		case 0xfcd3:
			mod = "Tecra 730XCDT";
			break;
		case 0xfcd4:
			mod = "Tecra 510x";
			break;
		case 0xfcd5:
			mod = "Satellite 200x";
			break;
		case 0xfcd7:
			mod = "Satellite Pro 430x";
			break;
		case 0xfcd8:
			mod = "Tecra 740x";
			break;
		case 0xfcd9:
			mod = "Portage 660CDT";
			break;
		case 0xfcda:
			mod = "Tecra 730CDT";
			break;
		case 0xfcdb:
			mod = "Portage 620CT";
			break;
		case 0xfcdc:
			mod = "Portage 650CT";
			break;
		case 0xfcdd:
			mod = "Satellite 110x";
			break;
		case 0xfcdf:
			mod = "Tecra 500x";
			break;
		case 0xfce0:
			mod = "Tecra 780DVD";
			break;
		case 0xfce2:
			mod = "Satellite 300x";
			break;
		case 0xfce3:
			mod = "Satellite 310x";
			break;
		case 0xfce4:
			mod = "Satellite Pro 490x";
			break;
		case 0xfce5:
			mod = "Libretto 100CT";
			break;
		case 0xfce6:
			mod = "Libretto 70CT";
			break;
		case 0xfce7:
			mod = "Tecra 540x/550x";
			break;
		case 0xfce8:
			mod = "Satellite Pro 470x/480x";
			break;
		case 0xfce9:
			mod = "Tecra 750DVD";
			break;
		case 0xfcea:
			mod = "Libretto 60";
			break;
		case 0xfceb:
			mod = "Libretto 50CT";
			break;
		case 0xfcec:
			mod = "Satellite 320x/330x/2500CDS";
			break;
		case 0xfced:
			mod = "Tecra 520x/530x";
			break;
		case 0xfcef:
			mod = "Satellite 220x/Pro 440x/460x";
			break;
		case 0xfcf3:
			mod = "Satellite 2450";
			break;
		case 0xfcf5:
			mod = "Satellite 5200";
			break;
		case 0xfcf7:
			mod = "Satellite Pro A10";
			break;
		case 0xfcf8:
			mod = "Satellite A20 Series";
			break;
		case 0xfcff:
			mod = "Tecra M2";
			break;
		default:
			mod = i18n("UNKNOWN");
			KMessageBox::information(0, i18n("Please send the model name and this id 0x%1 to:\n"
									 "coproscefalo@gmail.com").arg(id, 0, 16), i18n("Model Name"), "ModelID");
	}

	return mod;
};

#endif // MODEL_ID_H

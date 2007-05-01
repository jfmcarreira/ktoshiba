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

#ifndef TOSHIBA_FN_ACTIONS_H
#define TOSHIBA_FN_ACTIONS_H

#include "fnactions.h"

class KToshibaSMMInterface;

/**
 * @short Performs the Fn assosiated action
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.3
 */
class ToshibaFnActions : public FnActions
{
public:
    ToshibaFnActions(QWidget *parent = 0);
    virtual ~ToshibaFnActions();

    void performFnAction(int, int);
    KToshibaSMMInterface *m_Driver;
    bool m_SCIface;
    bool m_HCIface;
    int m_IFaceErr;
    int m_BIOS;
    int m_BIOSDate;
    int m_MachineID;
    int m_BatType;
private:
    void toggleBSM();
    void toggleVideo();
    void brightDown();
    void brightUp();
    void toggleWireless();
    void toggleMousePad();
    void toggleSpeakerVolume();
    void toggleFan();
    void toggleBootMethod();
    void toogleBackLight();
    void toggleBluetooth();
    void toggleEthernet();
    void initSCI();
    int m_BootType;
    int m_Vol;
    int m_Boot;
    int m_LANCtrl;
};

#endif // TOSHIBA_FN_ACTIONS_H

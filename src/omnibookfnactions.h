/***************************************************************************
 *   Copyright (C) 2006 by Azael Avalos                                    *
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

#ifndef OMNIBOOK_FN_ACTIONS_H
#define OMNIBOOK_FN_ACTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fnactions.h"

class KToshibaOmnibookInterface;

/**
 * @short Performs the Fn assosiated action
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.2
 */
class OmnibookFnActions : public FnActions
{
public:
    OmnibookFnActions(QWidget *parent = 0);
    virtual ~OmnibookFnActions();

    void performFnAction(int, int);
    KToshibaOmnibookInterface *m_Omni;
    QString m_ModelName;
    bool m_OmnibookIface;
    int m_ECType;
    int m_Video;
    int m_Bright;
    int m_Pad;
private:
    void toggleWireless();
    void toggleMousePad();
    void toggleFan();
    void toogleBackLight();
    void toggleBluetooth();
    int m_Snd;
    int m_Wireless;
    int m_Fan;
};

#endif // OMNIBOOK_FN_ACTIONS_H

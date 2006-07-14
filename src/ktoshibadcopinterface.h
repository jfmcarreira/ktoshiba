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

#ifndef KTOSHIBA_DCOPINTERFACE_H
#define KTOSHIBA_DCOPINTERFACE_H

#include <qobject.h>

#include <dcopobject.h>

/**
 * @short KToshiba DCOP Interafce
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.1
 */
class KToshibaDCOPInterface : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP
k_dcop:
    /**
     * This method can be called via DCOP to give a keycode.
     */
    ASYNC hotkey(int keycode);
public:
    KToshibaDCOPInterface(QObject *parent = 0, const char *name = 0);
    ~KToshibaDCOPInterface();
signals:
    void signalHotKey(int keycode);
};

#endif // KTOSHIBA_DCOPINTERFACE_H

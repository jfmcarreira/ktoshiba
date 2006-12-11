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

#ifndef SUSPEND_H
#define SUSPEND_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>

#include "ktoshibadbusinterface.h"

class QWidget;

/**
 * @short Suspend to RAM/Disk
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.2
 */
class Suspend : public QObject
{
    Q_OBJECT
public:
    Suspend(QWidget *parent = 0);
    ~Suspend();

    void toRAM();
    void toDisk();
    bool suspended;
    bool resumed;
signals:
    void resumedFromSTD();
    void setSuspendToDisk();
protected slots:
    void processMessage(msg_type, QString);
    void recheckDaemon();
    bool checkDaemon();
private:
    KToshibaDBUSInterface *m_DBUSIFace;
    QWidget *m_Parent;
    QString m_Info;
    bool powersaved_terminated;
    bool dbus_terminated;
};

#endif // SUSPEND_H

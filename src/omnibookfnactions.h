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

#include <qobject.h>

class QWidget;

class KToshibaProcInterface;
class StatusWidget;
class Suspend;

/**
 * @short Performs the Fn assosiated action
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.1
 */
class OmnibookFnActions : public QObject
{
    Q_OBJECT
public:
    /**
    * Default Constructor
    */
    OmnibookFnActions(QWidget *parent = 0);
    void hideWidgets();
    void performFnAction(int action);
    QString m_ModelName;
    bool m_OmnibookIface;
    int m_ECType;
    int m_Popup;
    int m_Video;
    int m_Bright;
    int m_Pad;
    int m_Mousepad;

    /**
    * Default Destructor
    */
    virtual ~OmnibookFnActions();
private:
    KToshibaProcInterface *m_Proc;
    StatusWidget *m_StatusWidget;
    Suspend *m_Suspend;
    void toggleMute();
    void lockScreen();
    void suspendToRAM();
    void suspendToDisk();
    void toggleMousePad();
    void toggleFan();
    void toogleBackLight();
    int m_Snd;
    int m_Fan;
};

#endif // OMNIBOOK_FN_ACTIONS_H

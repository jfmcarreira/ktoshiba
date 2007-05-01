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

#ifndef FN_ACTIONS_H
#define FN_ACTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>
#include <qpixmap.h>

class SettingsWidget;
class StatusWidget;
class CmdWidget;
class Suspend;

#ifdef ENABLE_SYNAPTICS
#include <synaptics/synaptics.h>

class Synaptics::Pad;

using namespace Synaptics;
#endif // ENABLE_SYNAPTICS

enum popuptype {
	Bluetooth,
	Ethernet,
	Wireless
};

/**
 * @short Shared Fn action class
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.1
 */
class FnActions : public QObject
{
    Q_OBJECT
public:
    FnActions(QWidget *parent = 0);
    virtual ~FnActions();

    void showWidget(int, int);
    void updateWidget(int, int, int extra = 0);
    void hideWidgets();
    void toggleMute(int *);
    void lockScreen();
    void runCommand(int);
    void showPassiveMsg(int, popuptype);
    SettingsWidget *m_SettingsWidget;
    StatusWidget *m_StatusWidget;
    Suspend *m_Suspend;
#ifdef ENABLE_SYNAPTICS
    Pad *mSynPad;
#endif // ENABLE_SYNAPTICS
    int m_Popup;
    int m_BatSave;
    int m_Video;
    int m_Bright;
    int m_Pad;
protected:
    int m_Snd;
    int m_Wireless;
    int m_Fan;
private slots:
    void saveCmd();
private:
    CmdWidget *m_CmdWidget;
    QWidget *m_Parent;
    QString m_Title;
    QString m_Text;
    QString m_Activated;
    QString m_Deactivated;
    QString m_NotSupported;
    QPixmap m_Icon;
    int m_Duration;
    int m_FnKey;
};

#endif // FN_ACTIONS_H

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
class Suspend;

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
    void showPassiveMsg(int state, char type);
    SettingsWidget *m_SettingsWidget;
    StatusWidget *m_StatusWidget;
    Suspend *m_Suspend;
    int m_Popup;
private:
    QWidget *m_Parent;
    QString m_Title;
    QString m_Text;
    QString m_Activated;
    QString m_Deactivated;
    QString m_NotSupported;
    QPixmap m_Icon;
    int m_Duration;
};

#endif // FN_ACTIONS_H

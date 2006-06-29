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

#include "omnibookfnactions.h"
#include "ktoshibaprocinterface.h"

#include <qwidgetstack.h>
#include <qapplication.h>

#include <kdebug.h>

#include "statuswidget.h"

OmnibookFnActions::OmnibookFnActions(QWidget *parent)
    : QObject( parent ),
      m_Proc( 0 )
{
    m_Proc = new KToshibaProcInterface(this);
    m_StatusWidget = new StatusWidget(0, "Screen Indicator", Qt::WX11BypassWM);
    m_StatusWidget->setFocusPolicy(QWidget::NoFocus);

    m_OmnibookIface = m_Proc->checkOmnibook();
    if (m_OmnibookIface) {
        m_Bright = m_Proc->omnibookGetBrightness();
        m_Fan = m_Proc->omnibookGetFan();
        m_Video = m_Proc->omnibookGetVideo();
        m_Parent = parent;
        m_Popup = 0;
    }
    else {
        m_Bright = -1;
        m_Fan = -1;
        m_Video = -1;
        m_Parent = parent;
        m_Popup = 0;
    }
}

OmnibookFnActions::~OmnibookFnActions()
{
    delete m_Parent; m_Parent = NULL;
    delete m_StatusWidget; m_StatusWidget = NULL;
    delete m_Proc; m_Proc = NULL;
}

void OmnibookFnActions::performFnAction(int action)
{
    switch(action) {
        case 1: // Raise/Lower Brightness
            if (m_Popup == 0) {
                QRect r = QApplication::desktop()->geometry();
                m_StatusWidget->move(r.center() - 
                    QPoint(m_StatusWidget->width()/2, m_StatusWidget->height()/2));
                m_StatusWidget->show();
                m_Popup = 1;
            }
            if (m_Popup == 1)
                break;
    }

    if (action == 1)
        if (m_Bright <= 7 && m_Bright >= 0)
            m_StatusWidget->wsStatus->raiseWidget(m_Bright + 4);
}


#include "omnibookfnactions.moc"

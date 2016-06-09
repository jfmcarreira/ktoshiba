/*
   Copyright (C) 2004-2016  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#include <QActionGroup>
#include <QIcon>
#include <QMenu>
#include <QStandardPaths>
#include <QTimer>

#include <KNotification>
#include <KLocalizedString>
#include <KProcess>

#include "fnactions.h"
#include "ktoshiba.h"
#include "ktoshiba_debug.h"

KToshiba::KToshiba()
    : KStatusNotifierItem(),
      m_fn(new FnActions(this))
{
    setTitle(i18n("KToshiba"));
    setIconByName(QStringLiteral("ktoshiba"));
    setToolTip(QStringLiteral("ktoshiba"), i18n("KToshiba"), i18n("Function (FN) key monitoring for Toshiba laptops"));
    setCategory(Hardware);
    setStatus(Passive);

    m_popupMenu = contextMenu();
    setAssociatedWidget(m_popupMenu);

    m_configure = m_popupMenu->addAction(i18n("Configure"));
    m_configure->setIcon(QIcon::fromTheme(QStringLiteral("configure")));

    connect(m_configure, SIGNAL(triggered()), this, SLOT(configureClicked()));
    connect(m_fn, SIGNAL(vibrationDetected()), this, SLOT(notifyHDDMovement()));
}

KToshiba::~KToshiba()
{
    delete m_configure; m_configure = NULL;
    delete m_fn; m_fn = NULL;
}

bool KToshiba::initialize()
{
    return m_fn->init();
}

void KToshiba::notifyHDDMovement()
{
    m_notification =
        KNotification::event(KNotification::Notification, i18n("KToshiba - HDD Monitor"),
                             i18n("Vibration has been detected and the HDD has been stopped to prevent damage"),
                             QIcon::fromTheme(QStringLiteral("drive-harddisk")).name(), 0, KNotification::CloseOnTimeout);
}

void KToshiba::notifyZoomDisabled()
{
    m_notification =
        KNotification::event(KNotification::Notification, i18n("KToshiba - Zoom Effect"),
                             i18n("Zoom effect is not supported or is disabled, Zoom actions cannot be activated"),
                             QIcon::fromTheme(QStringLiteral("ktoshiba")).name(), 0, KNotification::CloseOnTimeout);
}

void KToshiba::configureClicked()
{
    KProcess p;
    p.setProgram(QStandardPaths::findExecutable(QStringLiteral("kcmshell5")), QStringList(QStringLiteral("ktoshibam")));
    p.startDetached();
}

void KToshiba::changeStatus()
{
    setStatus(Passive);
}

void KToshiba::showApplication()
{
    setStatus(NeedsAttention);
    QTimer::singleShot(4000, this, SLOT(changeStatus()));
}

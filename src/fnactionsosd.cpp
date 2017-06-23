/*
 * Copyright (C) 2004-2016  Azael Avalos <coproscefalo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QDBusConnection>

#include <KDeclarative/QmlObject>
#include <KPackage/PackageLoader>

#include "fnactionsosd.h"
#include "ktoshiba_debug.h"

#define SERVICE QLatin1Literal("org.kde.plasmashell")
#define PATH QLatin1Literal("/org/kde/osdService")
#define CONNECTION QDBusConnection::sessionBus()


FnActionsOsd::FnActionsOsd(QObject *parent)
    : QObject(parent),
		m_osdService(SERVICE, PATH, CONNECTION)
{
}

void FnActionsOsd::touchpadEnabledChanged(bool touchpadEnabled)
{
		m_osdService.touchpadEnabledChanged(touchpadEnabled);
}

void FnActionsOsd::showText(const QString& icon, const QString& text)
{
		m_osdService.showText(icon, text);
}



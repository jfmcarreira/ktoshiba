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

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

#include "fnactionsosd.h"
#include "ktoshiba_debug.h"

#define DESTINATION QLatin1Literal("org.kde.plasmashell")
#define PATH QLatin1Literal("/org/kde/osdService")
#define INTERFACE QLatin1Literal("org.kde.osdService")
#define CONNECTION QDBusConnection::sessionBus()


FnActionsOsd::FnActionsOsd(QObject *parent)
		: QObject(parent)
{
}

void FnActionsOsd::touchpadEnabledChanged(bool touchpadEnabled)
{
	QDBusMessage msg = QDBusMessage::createMethodCall(
			DESTINATION, PATH, INTERFACE,
			QLatin1Literal("touchpadEnabledChanged")
	);
	msg << touchpadEnabled;
  QDBusConnection::sessionBus().asyncCall(msg);
}

void FnActionsOsd::showText(const QString& icon, const QString& text)
{
	QDBusMessage msg = QDBusMessage::createMethodCall(
			DESTINATION, PATH, INTERFACE,
			QLatin1Literal("showText")
	);
	msg << icon << text;
	QDBusConnection::sessionBus().asyncCall(msg);
}



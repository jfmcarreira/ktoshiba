/*
   Copyright (C) 2004-2011  Azael Avalos <coproscefalo@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QtDBus/QDBusReply>
#include <QString>
#include <QDBusVariant>

#include <KDebug>

#include "ktoshibadbusinterface.h"

QString Introspect = "org.freedesktop.DBus.Introspectable";

KToshibaDBusInterface::KToshibaDBusInterface(QObject *parent)
    : QObject( parent ),
      m_mediaPlayer( NoMP ),
      m_devilIface( NULL ),
      m_touchpadIface( NULL ),
      m_hibernate( false ),
      m_suspend( false ),
      m_str( 2 ),
      m_std( 4 )
{
    m_devilIface = new QDBusInterface("org.kde.Solid.PowerManagement",
			       "/org/kde/Solid/PowerManagement",
			       "org.kde.Solid.PowerManagement",
			       QDBusConnection::sessionBus(), this);
    if (!m_devilIface->isValid()) {
        QDBusError err(m_devilIface->lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return;
    }

    m_touchpadIface = new QDBusInterface("org.kde.synaptiks",
			       "/TouchpadManager",
			       "org.kde.TouchpadManager",
			       QDBusConnection::sessionBus(), this);
    if (!m_touchpadIface->isValid()) {
        QDBusError err(m_touchpadIface->lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return;
    }

    checkCompositeStatus();

    connect( m_devilIface, SIGNAL( profileChanged(QString) ),
	     this, SLOT( profileChangedSlot(QString) ) );
    connect( m_touchpadIface, SIGNAL( touchpadSwitched(bool, QString, QDBusVariant) ),
	     this, SLOT( touchpadChangedSlot(bool, QString, QDBusVariant) ) );
}

KToshibaDBusInterface::~KToshibaDBusInterface()
{
    delete m_devilIface; m_devilIface = NULL;
}

void KToshibaDBusInterface::profileChangedSlot(QString profile)
{
    // Only emit signal if profile is one of our supported states
    if (profile == "Performance" || profile == "Powersave" || profile == "Presentation")
        emit profileChanged(profile);

    return;
}

void KToshibaDBusInterface::touchpadChangedSlot(bool on, QString reason, QDBusVariant closure)
{
    kDebug() << "TouchPad changed, reason: " << reason << endl
             << " (" << closure.variant() << ")" << endl;
    emit touchpadChanged(on);
}

void KToshibaDBusInterface::checkCompositeStatus()
{
    QDBusInterface iface("org.kde.kwin",
			 "/KWin",
			 "org.kde.KWin",
			 QDBusConnection::sessionBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        m_compositeEnabled = false;
        return;
    }

    QDBusReply<bool> reply = iface.call("compositingActive");

    if (!reply.isValid()) {
        QDBusError err(reply.error());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        m_compositeEnabled = false;
        return;
    }

    m_compositeEnabled = reply.value();
}

void KToshibaDBusInterface::lockScreen()
{
    QDBusInterface iface("org.freedesktop.ScreenSaver",
			 "/ScreenSaver",
			 "org.freedesktop.ScreenSaver",
			 QDBusConnection::sessionBus(), 0);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
        return;
    }

    QDBusReply<void> reply = iface.call("Lock");

    if (!reply.isValid()) {
        QDBusError err(iface.lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }
}

void KToshibaDBusInterface::setProfile(QString profile)
{
    QDBusReply<void> reply = m_devilIface->call("setProfile", profile);

    if (!reply.isValid()) {
        QDBusError err(m_devilIface->lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }
}

bool KToshibaDBusInterface::hibernate()
{
    if (m_hibernate) {
        QDBusReply<void> reply = m_devilIface->call("suspend", m_std);

        if (!reply.isValid()) {
            QDBusError err(m_devilIface->lastError());
            kError() << err.name() << endl
                     << "Message: " << err.message() << endl;

            return false;
        }

        return true;
    }

    return false;
}

bool KToshibaDBusInterface::suspend()
{
    if (m_hibernate) {
        QDBusReply<void> reply = m_devilIface->call("suspend", m_str);

        if (!reply.isValid()) {
            QDBusError err(m_devilIface->lastError());
            kError() << err.name() << endl
                     << "Message: " << err.message() << endl;

            return false;
        }

        return true;
    }

    return false;
}

void KToshibaDBusInterface::setTouchPad(bool on)
{
    QDBusReply<void> reply = m_touchpadIface->call("setTouchpadOn", on, "Hotkey");

    if (!reply.isValid()) {
        QDBusError err(m_touchpadIface->lastError());
        kError() << err.name() << endl
                 << "Message: " << err.message() << endl;
    }
}

bool KToshibaDBusInterface::checkMediaPlayer()
{
    QString Service;

    switch (m_mediaPlayer) {
        case Amarok:
            Service = "org.kde.amarok";
            break;
        case Kaffeine:
            Service = "org.kde.kaffeine";
            break;
        case JuK:
            Service = "org.kde.juk";
            break;
    }
    QDBusInterface iface(Service, "/", Introspect,
                         QDBusConnection::sessionBus(), 0);

    if (!iface.isValid())
        return false;

    return true;
}

void KToshibaDBusInterface::playerPlayPause()
{
    switch (m_mediaPlayer) {
        case Amarok:
            message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "PlayPause");
            break;
        case Kaffeine:
            // UGLY: Kaffeine doesn't support Play/Pause (yet...)
            // so let's use this instead to see if something's playing...
            {QDBusInterface iface("org.kde.kaffeine", "/Player",
					"org.freedesktop.MediaPlayer", QDBusConnection::sessionBus(), 0);
            if (!iface.isValid()) {
                QDBusError err(iface.lastError());
                kError() << err.name() << endl
                         << "Message: " << err.message() << endl;
                return;
            }

            QDBusReply<int> reply = iface.call("PositionGet");

            if (!reply.isValid()) {
                QDBusError err(iface.lastError());
                kError() << err.name() << endl
                         << "Message: " << err.message() << endl;
                return;
            }
            if (reply.value() == 0)
                message = QDBusMessage::createMethodCall("org.kde.kaffeine",
                                                         "/Player",
                                                         "org.freedesktop.MediaPlayer",
                                                         "Play");
            else
                message = QDBusMessage::createMethodCall("org.kde.kaffeine",
                                                         "/Player",
                                                         "org.freedesktop.MediaPlayer",
                                                         "Pause");
            }
            break;
        case JuK:
            message = QDBusMessage::createMethodCall("org.kde.juk",
                                                     "/Player",
                                                     "org.kde.juk.player",
                                                     "playPause");
            break;
    }

    QDBusConnection::sessionBus().send(message);
}

void KToshibaDBusInterface::playerStop()
{
    switch (m_mediaPlayer) {
        case Amarok:
            message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Stop");
            break;
        case Kaffeine:
            message = QDBusMessage::createMethodCall("org.kde.kaffeine",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Stop");
            break;
        case JuK:
            message = QDBusMessage::createMethodCall("org.kde.juk",
                                                     "/Player",
                                                     "org.kde.juk.player",
                                                     "stop");
            break;
    }

    QDBusConnection::sessionBus().send(message);
}

void KToshibaDBusInterface::playerPrevious()
{
    switch (m_mediaPlayer) {
        case Amarok:
            message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Prev");
            break;
        case Kaffeine:
            message = QDBusMessage::createMethodCall("org.kde.kaffeine",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Prev");
            break;
        case JuK:
            message = QDBusMessage::createMethodCall("org.kde.juk",
                                                     "/Player",
                                                     "org.kde.juk.player",
                                                     "back");
            break;
    }

    QDBusConnection::sessionBus().send(message);
}

void KToshibaDBusInterface::playerNext()
{
    switch (m_mediaPlayer) {
        case Amarok:
            message = QDBusMessage::createMethodCall("org.kde.amarok",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Next");
            break;
        case Kaffeine:
            message = QDBusMessage::createMethodCall("org.kde.kaffeine",
                                                     "/Player",
                                                     "org.freedesktop.MediaPlayer",
                                                     "Next");
            break;
        case JuK:
            message = QDBusMessage::createMethodCall("org.kde.juk",
                                                     "/Player",
                                                     "org.kde.juk.player",
                                                     "forward");
            break;
    }

    QDBusConnection::sessionBus().send(message);
}


#include "ktoshibadbusinterface.moc"

/*
   Copyright (C) 2004-2009  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef KTOSHIBA_DBUS_INTERFACE_H
#define KTOSHIBA_DBUS_INTERFACE_H

#include <QObject>
#include <QString>
#include <QtDBus/QDBusMessage>

class QDBusInterface;

class KToshibaDBusInterface : public QObject
{
    Q_OBJECT

public:
    enum MediaPlayer {
        Amarok,
        Kaffeine
    };
    
public:
    KToshibaDBusInterface(QObject *parent);
    ~KToshibaDBusInterface();

    QString getModel();
    void lockScreen();
    void setProfile(QString);
    bool hibernate();
    bool suspend();
    int getBrightness();
    //void setMediaplayer(MediaPlayer);
    void launchMediaPlayer();
    void playerPlayPause();
    void playerStop();
    void playerPrevious();
    void playerNext();

Q_SIGNALS:
    void hotkeyPressed(QString);
    void profileChanged(QString, QStringList);

private Q_SLOTS:
    void gotInputEvent(QString, QString);
    void profileChangedSlot(QString, QStringList);

private:
    void checkSupportedSuspend();
    bool checkMediaPlayer();
    int showMessageBox();

    QDBusInterface* m_inputIface;
    QDBusInterface* m_kbdIface;
    QDBusInterface* m_devilIface;
    QDBusMessage message;
    bool m_hibernate;
    bool m_suspend;
    int m_str;
    int m_std;
    //int m_player;
};

#endif	// KTOSHIBA_DBUS_INTERFACE_H

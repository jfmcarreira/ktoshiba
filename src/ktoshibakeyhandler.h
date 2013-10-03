/*
   Copyright (C) 2013  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef KTOSHIBAKEYHANDLER_H
#define KTOSHIBAKEYHANDLER_H

class QSocketNotifier;

class KToshibaKeyHandler : public QObject
{
    Q_OBJECT

public:
    KToshibaKeyHandler(QObject *parent);
    ~KToshibaKeyHandler();

Q_SIGNALS:
    void hotkeyPressed(int);

private Q_SLOTS:
    void readData();

private:
    QString findDevice();

    QSocketNotifier *m_Notifier;
    QString m_Device;

    int m_Fd;
};

#endif

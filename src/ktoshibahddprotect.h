/*
   Copyright (C) 2014-2015  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef KTOSHIBAHDDPROTECT_H
#define KTOSHIBAHDDPROTECT_H

class QSocketNotifier;

class HelperActions;

class KToshibaHDDProtect : public QObject
{
    Q_OBJECT

public:
    KToshibaHDDProtect(QObject *parent);
    ~KToshibaHDDProtect();

    void setHDDProtection(bool enabled);

private Q_SLOTS:
    void protectHDD(int);

Q_SIGNALS:
    void movementDetected();

private:
    int hddEventSocket();
    void unloadHDDHeads(int event);

    HelperActions *m_helper;
    QSocketNotifier *m_notifier;
    int m_fd;
};

#endif // KTOSHIBAHDDPROTECT_H

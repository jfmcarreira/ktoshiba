/*
   Copyright (C) 2014-2015  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef KTOSHIBANETLINKEVENTS_H
#define KTOSHIBANETLINKEVENTS_H

#include <QString>

class QSocketNotifier;

class KToshibaNetlinkEvents : public QObject
{
    Q_OBJECT

public:
    KToshibaNetlinkEvents(QObject *parent);
    ~KToshibaNetlinkEvents();

    bool attach();
    void detach();
    void setDeviceHID(QString);

Q_SIGNALS:
    void hapsEvent(int);
    void tvapEvent(int);

private Q_SLOTS:
    void parseEvents(int);

private:
    QSocketNotifier *m_notifier;
    QString m_deviceHID;
    int m_socket;
};

#endif // KTOSHIBANETLINKEVENTS_H

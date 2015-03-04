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

#ifndef KTOSHIBAHDDPROTECT_H
#define KTOSHIBAHDDPROTECT_H

#define HDD_VIBRATED   0x80
#define HDD_STABILIZED 0x81

class QSocketNotifier;

class KToshibaHDDProtect : public QObject
{
    Q_OBJECT

public:
    KToshibaHDDProtect(QObject *parent);
    ~KToshibaHDDProtect();

    bool attach();
    void detach();
    void setHDDProtection(bool);

private Q_SLOTS:
    void parseEvents(int);

Q_SIGNALS:
    void eventDetected(int);

private:
    QSocketNotifier *m_notifier;
    int m_socket;
};

#endif // KTOSHIBAHDDPROTECT_H

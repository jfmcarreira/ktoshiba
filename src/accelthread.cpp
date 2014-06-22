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

#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QStringList>

#include <KDebug>

#include "accelthread.h"

AccelThread::AccelThread(QObject *parent)
    : QThread(parent)
{
}

void AccelThread::run()
{
    QTimer timer;
    connect( &timer, SIGNAL( timeout() ), this, SLOT( getAxes() ) );
    timer.start(50); // TODO: Make this value configurable...
    exec();
}

void AccelThread::getAxes()
{
    QFile file("/sys/devices/platform/toshiba/position");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kError() << "" << endl;
        return;
    }

    QTextStream stream(&file);
    QString axes = stream.readAll();
    file.close();

    QStringList data = axes.split(" ");
    x = data[0].toInt();
    y = data[1].toInt();
    z = data[2].toInt();

    emit axesUpdated(x, y, z);
}


#include "accelthread.moc"

/*
   Copyright (C) 2014  Azael Avalos <coproscefalo@gmail.com>

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
#include <QDebug>
#include <QVariantMap>

extern "C" {
#include <asm/errno.h>
}

#include "ktoshhelper.h"

KToshHelper::KToshHelper(QObject *parent)
    : QObject(parent)
{
    m_device = findDevicePath();
    if (m_device.isEmpty() || m_device.isNull())
        exit(-1);
}

QString KToshHelper::findDevicePath()
{
    QFile file(this);

    file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS1900:00/path");
    if (file.exists()) {
        qDebug() << "Found interface TOS1900" << endl;
        return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS1900:00/");
    }

    file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6200:00/path");
    if (file.exists()) {
        qDebug() << "Found interface TOS6200" << endl;
        return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6200:00/");
    }

    file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6208:00/path");
    if (file.exists()) {
        qDebug() << "Found interface TOS6208" << endl;
        return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6208:00/");
    }

    qWarning() << "No known interface found" << endl;

    return QString("");
}

ActionReply KToshHelper::toggletouchpad(QVariantMap args)
{
    Q_UNUSED(args)

    ActionReply reply;

    QFile file(m_device + "touchpad");
    if (!file.exists()) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-ENODEV);
        reply.setErrorDescription("The touchpad device does not exist");

        return reply;
    }

    if (!file.open(QIODevice::ReadWrite)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(file.error());

       return reply;
    }

    QTextStream stream(&file);
    int state = stream.readAll().toInt();
    stream << !state;
    file.close();

    reply = ActionReply::Success;

    return reply;
}

ActionReply KToshHelper::setillumination(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-EINVAL);
        reply.setErrorDescription("The value was out of range");

        return reply;
    }

    QFile file("/sys/class/leds/toshiba::illumination/brightness");
    if (!file.exists()) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-ENODEV);
        reply.setErrorDescription("The illumination led does not exist");

        return reply;
    }

    if (!file.open(QIODevice::WriteOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(file.error());

       return reply;
    }

    QTextStream stream(&file);
    stream << state;
    file.close();

    reply = ActionReply::Success;

    return reply;
}

ActionReply KToshHelper::seteco(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-EINVAL);
        reply.setErrorDescription("The value was out of range");

        return reply;
    }

    QFile file("/sys/class/leds/toshiba::eco_mode/brightness");
    if (!file.exists()) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-ENODEV);
        reply.setErrorDescription("The eco mode led does not exist");

        return reply;
    }

    if (!file.open(QIODevice::WriteOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(file.error());

       return reply;
    }

    QTextStream stream(&file);
    stream << state;
    file.close();

    reply = ActionReply::Success;

    return reply;
}

ActionReply KToshHelper::kbdmode(QVariantMap args)
{
    Q_UNUSED(args)

    ActionReply reply;

    QFile file(m_device + "kbd_backlight_mode");
    if (!file.exists()) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-ENODEV);
        reply.setErrorDescription("The keyboard backlight mode device does not exist");

        return reply;
    }

    if (!file.open(QIODevice::ReadOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(file.error());

       return reply;
    }

    QTextStream stream(&file);
    int mode = stream.readAll().toInt();
    file.close();

    reply = ActionReply::Success;
    reply.addData("mode", mode);

    return reply;
}

ActionReply KToshHelper::setkbdmode(QVariantMap args)
{
    ActionReply reply;
    int mode = args["mode"].toInt();

    if (mode < 1 || mode > 2) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-EINVAL);
        reply.setErrorDescription("The value was out of range");

        return reply;
    }

    QFile file(m_device + "kbd_backlight_mode");
    if (!file.exists()) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-ENODEV);
        reply.setErrorDescription("The keyboard backlight mode device does not exist");

        return reply;
    }

    if (!file.open(QIODevice::WriteOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(file.error());

       return reply;
    }

    QTextStream stream(&file);
    stream << mode;
    file.close();

    reply = ActionReply::Success;

    return reply;
}

ActionReply KToshHelper::kbdtimeout(QVariantMap args)
{
    Q_UNUSED(args)

    ActionReply reply;

    QFile file(m_device + "kbd_backlight_timeout");
    if (!file.exists()) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-ENODEV);
        reply.setErrorDescription("The keyboard backlight timeout device does not exist");

        return reply;
    }

    if (!file.open(QIODevice::ReadOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(file.error());

       return reply;
    }

    QTextStream stream(&file);
    int time = stream.readAll().toInt();
    file.close();

    reply = ActionReply::Success;
    reply.addData("time", time);

    return reply;
}

ActionReply KToshHelper::setkbdtimeout(QVariantMap args)
{
    ActionReply reply;
    int time = args["time"].toInt();

    if (time < 0 || time > 60) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-EINVAL);
        reply.setErrorDescription("The value was out of range");

        return reply;
    }

    QFile file(m_device + "kbd_backlight_timeout");
    if (!file.exists()) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-ENODEV);
        reply.setErrorDescription("The keyboard backlight timeout device does not exist");

        return reply;
    }

    if (!file.open(QIODevice::WriteOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(file.error());

       return reply;
    }

    QTextStream stream(&file);
    stream << time;
    file.close();

    reply = ActionReply::Success;

    return reply;
}

KDE4_AUTH_HELPER_MAIN("net.sourceforge.ktoshiba.ktoshhelper", KToshHelper)

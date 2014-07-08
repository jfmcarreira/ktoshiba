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

#include <QDebug>
#include <QVariantMap>

#include "ktoshhelper.h"

KToshHelper::KToshHelper(QObject *parent)
    : QObject(parent)
{
    m_driverPath = findDriverPath();
    if (m_driverPath.isEmpty())
        exit(-1);
}

QString KToshHelper::findDriverPath()
{
    m_file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS1900:00/path");
    if (m_file.exists()) {
        qDebug() << "Found interface TOS1900" << endl;
        return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS1900:00/");
    }

    m_file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6200:00/path");
    if (m_file.exists()) {
        qDebug() << "Found interface TOS6200" << endl;
        return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6200:00/");
    }

    m_file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6208:00/path");
    if (m_file.exists()) {
        qDebug() << "Found interface TOS6208" << endl;
        return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS6208:00/");
    }

    qWarning() << "No known interface found" << endl;

    return QString("");
}

ActionReply KToshHelper::deviceexists(QVariantMap args)
{
    ActionReply reply;
    QString device = args["device"].toString();

    m_file.setFileName(m_driverPath + device);
    if (!m_file.exists()) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-19); // No such device
        reply.setErrorDescription("The device does not exist");

        return reply;
    }

    return reply;
}

ActionReply KToshHelper::leddeviceexists(QVariantMap args)
{
    ActionReply reply;
    QString device = args["device"].toString();

    m_file.setFileName("/sys/class/leds/toshiba::" + device + "/brightness");
    if (!m_file.exists()) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-19); // No such device
        reply.setErrorDescription("The device does not exist");

        return reply;
    }

    return reply;
}

ActionReply KToshHelper::toggletouchpad(QVariantMap args)
{
    Q_UNUSED(args)

    ActionReply reply;

    m_file.setFileName(m_driverPath + "touchpad");
    if (!m_file.open(QIODevice::ReadWrite)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(m_file.error());
       reply.setErrorDescription(m_file.errorString());

       return reply;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    stream << !state;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::illumination(QVariantMap args)
{
    Q_UNUSED(args)

    ActionReply reply;

    m_file.setFileName("/sys/class/leds/toshiba::illumination/brightness");
    if (!m_file.open(QIODevice::ReadOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(m_file.error());
       reply.setErrorDescription(m_file.errorString());

       return reply;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    reply.addData("state", state);

    return reply;
}

ActionReply KToshHelper::setillumination(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-22); // Invalid argument
        reply.setErrorDescription("The value was out of range");

        return reply;
    }

    m_file.setFileName("/sys/class/leds/toshiba::illumination/brightness");
    if (!m_file.open(QIODevice::WriteOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(m_file.error());
       reply.setErrorDescription(m_file.errorString());

       return reply;
    }

    QTextStream stream(&m_file);
    stream << state;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::eco(QVariantMap args)
{
    Q_UNUSED(args)

    ActionReply reply;

    m_file.setFileName("/sys/class/leds/toshiba::eco_mode/brightness");
    if (!m_file.open(QIODevice::ReadOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(m_file.error());
       reply.setErrorDescription(m_file.errorString());

       return reply;
    }

    QTextStream stream(&m_file);
    int state = stream.readAll().toInt();
    m_file.close();

    reply.addData("state", state);

    return reply;
}

ActionReply KToshHelper::seteco(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-22); // Invalid argument
        reply.setErrorDescription("The value was out of range");

        return reply;
    }

    m_file.setFileName("/sys/class/leds/toshiba::eco_mode/brightness");
    if (!m_file.open(QIODevice::WriteOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(m_file.error());
       reply.setErrorDescription(m_file.errorString());

       return reply;
    }

    QTextStream stream(&m_file);
    stream << state;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::kbdmode(QVariantMap args)
{
    Q_UNUSED(args)

    ActionReply reply;

    m_file.setFileName(m_driverPath + "kbd_backlight_mode");
    if (!m_file.open(QIODevice::ReadOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(m_file.error());
       reply.setErrorDescription(m_file.errorString());

       return reply;
    }

    QTextStream stream(&m_file);
    int mode = stream.readAll().toInt();
    m_file.close();

    reply.addData("mode", mode);

    return reply;
}

ActionReply KToshHelper::setkbdmode(QVariantMap args)
{
    ActionReply reply;
    int mode = args["mode"].toInt();

    if (mode < 1 || mode > 2) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-22); // Invalid argument
        reply.setErrorDescription("The value was out of range");

        return reply;
    }

    m_file.setFileName(m_driverPath + "kbd_backlight_mode");
    if (!m_file.open(QIODevice::WriteOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(m_file.error());
       reply.setErrorDescription(m_file.errorString());

       return reply;
    }

    QTextStream stream(&m_file);
    stream << mode;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::kbdtimeout(QVariantMap args)
{
    Q_UNUSED(args)

    ActionReply reply;

    m_file.setFileName(m_driverPath + "kbd_backlight_timeout");
    if (!m_file.open(QIODevice::ReadOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(m_file.error());
       reply.setErrorDescription(m_file.errorString());

       return reply;
    }

    QTextStream stream(&m_file);
    int time = stream.readAll().toInt();
    m_file.close();

    reply.addData("time", time);

    return reply;
}

ActionReply KToshHelper::setkbdtimeout(QVariantMap args)
{
    ActionReply reply;
    int time = args["time"].toInt();

    if (time < 0 || time > 60) {
        reply = ActionReply::HelperErrorReply;
        reply.setErrorCode(-22); // Invalid argument
        reply.setErrorDescription("The value was out of range");

        return reply;
    }

    m_file.setFileName(m_driverPath + "kbd_backlight_timeout");
    if (!m_file.open(QIODevice::WriteOnly)) {
       reply = ActionReply::HelperErrorReply;
       reply.setErrorCode(m_file.error());
       reply.setErrorDescription(m_file.errorString());

       return reply;
    }

    QTextStream stream(&m_file);
    stream << time;
    m_file.close();

    return reply;
}

KDE4_AUTH_HELPER_MAIN("net.sourceforge.ktoshiba.ktoshhelper", KToshHelper)

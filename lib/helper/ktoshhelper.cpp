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

#include <QDebug>
#include <QVariantMap>
#include <QDir>
#include <QStringList>
#include <QProcess>
#include <QByteArray>
#include <QStandardPaths>

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
    QStringList m_devices;
    m_devices << "TOS1900:00" << "TOS6200:00" << "TOS6207:00" << "TOS6208:00";
    QString path("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/path");
    for (int current = m_devices.indexOf(m_devices.first()); current <= m_devices.indexOf(m_devices.last());) {
        m_file.setFileName(path.arg(m_devices.at(current)));
        if (m_file.exists())
            return QString("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/").arg(m_devices.at(current));

        current++;
    }

    qWarning() << "No known interface found" << endl;

    return QString("");
}

ActionReply KToshHelper::dumpsysinfo(QVariantMap args)
{
    Q_UNUSED(args)

    ActionReply reply;

    QStringList paths = QStringList() << "/sbin" << "/usr/sbin" << "/usr/local/sbin";
    QString dmidecode = QStandardPaths::findExecutable("dmidecode", paths);
    if (dmidecode.isEmpty()) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "dmidecode binary not found, system info could not be parsed";

        return reply;
    }

    QProcess p;
    p.start(dmidecode);
    if (!p.waitForFinished(-1)) {
        reply = ActionReply::HelperErrorReply();
        qWarning() << "dmidecode failed with error code" << p.error();

        return reply;
    }

    QFile file("/var/tmp/dmidecode");
    if (!file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "dumpsysinfo failed with error code" << file.error() << file.errorString();

        return reply;
    }
    file.write(p.readAll());
    file.close();

    return reply;
}

ActionReply KToshHelper::settouchpad(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "touchpad");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply::HelperErrorReply();
        qWarning() << "settouchpad failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << state;
    m_file.close();

    // Code for TouchPad LED (when it is available), we simply uncomment this
    /*m_file.setFileName("/sys/class/leds/psmouse::synaptics/brightness");
    if (!m_file.exists()) {
        // No need to report as an error if the file does not exists
        return reply;
    } else {
        if (!state) {
            if (!m_file.open(QIODevice::WriteOnly)) {
                reply = ActionReply(ActionReply::HelperErrorType);
                qWarning() << "settouchpad failed with eror code" << m_file.error() << m_file.errorString();

                return reply;
            }

            QTextStream stream(&m_file);
            stream << !state;
            m_file.close();
        }
    }*/

    return reply;
}

ActionReply KToshHelper::setillumination(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName("/sys/class/leds/toshiba::illumination/brightness");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setillumination failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << state;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::seteco(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName("/sys/class/leds/toshiba::eco_mode/brightness");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "seteco failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << state;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::setkbdmode(QVariantMap args)
{
    ActionReply reply;
    int mode = args["mode"].toInt();

    if (mode != 1 && mode != 2 && mode != 8 && mode != 0x10) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "kbd_backlight_mode");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setkbdmode failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << mode;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::setkbdtimeout(QVariantMap args)
{
    ActionReply reply;
    int time = args["time"].toInt();

    if (time < 1 || time > 60) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "kbd_backlight_timeout");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setkbdtimeout failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << time;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::setusbsleepcharge(QVariantMap args)
{
    ActionReply reply;
    int mode = args["mode"].toInt();

    if (mode < 0 || mode > 2) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "usb_sleep_charge");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setusbsleepcharge failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << mode;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::setusbsleepfunctionsbatlvl(QVariantMap args)
{
    ActionReply reply;
    int level = args["level"].toInt();

    if (level < 0 || level > 100) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "sleep_functions_on_battery");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setusbsleepfunctionsbatlvl failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << level;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::setusbrapidcharge(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "usb_rapid_charge");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setusbrapidcharge failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << state;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::setusbsleepmusic(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "usb_sleep_music");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setusbsleepmusic failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << state;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::setkbdfunctions(QVariantMap args)
{
    ActionReply reply;
    int mode = args["mode"].toInt();

    if (mode < 0 || mode > 1) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "kbd_function_keys");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setkbdfunctions failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << mode;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::setpanelpoweron(QVariantMap args)
{
    ActionReply reply;
    int state = args["state"].toInt();

    if (state < 0 || state > 1) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "panel_power_on");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setpanelpoweron failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << state;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::setusbthree(QVariantMap args)
{
    ActionReply reply;
    int mode = args["mode"].toInt();

    if (mode < 0 || mode > 1) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName(m_driverPath + "usb_three");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setusbthree failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << mode;
    m_file.close();

    return reply;
}

/*
 * HDD Protection calls
 */
ActionReply KToshHelper::setprotectionlevel(QVariantMap args)
{
    ActionReply reply;
    int level = args["level"].toInt();

    if (level < 0 || level > 3) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    m_file.setFileName("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS620A:00/protection_level");
    if (!m_file.open(QIODevice::WriteOnly)) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "setprotectionlevel failed with error code" << m_file.error() << m_file.errorString();

        return reply;
    }

    QTextStream stream(&m_file);
    stream << level;
    m_file.close();

    return reply;
}

ActionReply KToshHelper::unloadheads(QVariantMap args)
{
    ActionReply reply;
    int timeout = args["timeout"].toInt();

    if (timeout != 0 && timeout != 5000) {
        reply = ActionReply(ActionReply::HelperErrorType);
        qWarning() << "The value was out of range";

        return reply;
    }

    QDir dir("/sys/block");
    dir.setNameFilters(QStringList("sd*"));
    QStringList hdds(dir.entryList());
    for (int current = hdds.indexOf(hdds.first()); current <= hdds.indexOf(hdds.last()); current++) {
        QString path("/sys/block/%1/device/unload_heads");
        m_file.setFileName(path.arg(hdds.at(current)));
        if (m_file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&m_file);
            stream << timeout;
            m_file.close();
        } else {
            qWarning() << "Could not protect" << hdds.at(current) << "heads";
            qWarning() << "Received error code" << m_file.error() << m_file.errorString();
        }
    }

    return reply;
}

KAUTH_HELPER_MAIN("net.sourceforge.ktoshiba.ktoshhelper", KToshHelper)

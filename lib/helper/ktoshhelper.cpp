/*
   Copyright (C) 2014-2016  Azael Avalos <coproscefalo@gmail.com>

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

#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTextStream>

#include "ktoshhelper.h"

KToshHelper::KToshHelper(QObject *parent)
    : QObject(parent)
{
    QDir dir("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS620A:00/");
    m_supported = dir.exists();
}

/*
 * HDD Protection calls
 */
ActionReply KToshHelper::setprotectionlevel(QVariantMap args)
{
    ActionReply reply;
    int level = args["level"].toInt();

    if (!m_supported) {
        reply = ActionReply::HelperErrorReply();
        //reply.setErrorCode(0x8000);
        reply.setErrorDescription("Feature is not supported");

        return reply;
    }

    if (level < 0 || level > 3) {
        reply = ActionReply::HelperErrorReply();
        //reply.setErrorCode(0x8300);
        reply.setErrorDescription("Invalid parameters");

        return reply;
    }

    QFile file("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/TOS620A:00/protection_level");
    if (!file.open(QIODevice::WriteOnly)) {
        reply = ActionReply::HelperErrorReply();
        //reply.setErrorCode(file.error());
        reply.setErrorDescription(file.errorString());

        return reply;
    }

    QTextStream stream(&file);
    stream << level;
    file.close();

    return reply;
}

ActionReply KToshHelper::unloadheads(QVariantMap args)
{
    ActionReply reply;
    int timeout = args["timeout"].toInt();

    if (!m_supported) {
        reply = ActionReply::HelperErrorReply();
        //reply.setErrorCode(0x8000);
        reply.setErrorDescription("Feature is not supported");

        return reply;
    }

    if (timeout != 0 && timeout != 5000) {
        reply = ActionReply::HelperErrorReply();
        //reply.setErrorCode(0x8300);
        reply.setErrorDescription("Invalid parameters");

        return reply;
    }

    QDir dir("/sys/block");
    dir.setNameFilters(QStringList("sd*"));
    QStringList drives(dir.entryList());
    QStringList hdds;
    QFile file;

    /*
     * Find which drives are HDDs, SSDs do not need protection
     */
    foreach (const QString &drive, drives) {
        file.setFileName(QString("/sys/block/%1/queue/rotational").arg(drive));
        if (!file.open(QIODevice::ReadOnly)) {
            reply = ActionReply::HelperErrorReply();
            //reply.setErrorCode(file.error());
            reply.setErrorDescription(file.errorString());

            return reply;
        }

        int rotational = file.readLine().simplified().toInt();
        file.close();

        if (rotational) {
            hdds << drive;
        }
    }
    
    /*
     * Protect the HDD(s)
     */
    foreach (const QString &hdd, hdds) {
        file.setFileName(QString("/sys/block/%1/device/unload_heads").arg(hdd));
        if (!file.open(QIODevice::WriteOnly)) {
            reply = ActionReply::HelperErrorReply();
            //reply.setErrorCode(file.error());
            reply.setErrorDescription(file.errorString());
        }

        QTextStream stream(&file);
        stream << timeout;
        file.close();
    }

    return reply;
}

KAUTH_HELPER_MAIN("net.sourceforge.ktoshiba.ktoshhelper", KToshHelper)

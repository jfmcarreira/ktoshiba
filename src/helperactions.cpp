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

#include <KAuth/Action>
#include <KDebug>

#include "helperactions.h"

#define HELPER_ID "net.sourceforge.ktoshiba.ktoshhelper"

HelperActions::HelperActions(QObject *parent)
    : QObject( parent )
{
    isTouchPadSupported = checkTouchPad();
    isIlluminationSupported = checkIllumination();
    isECOSupported = checkECO();
    isKBDBacklightSupported = checkKBDBacklight();
    isAccelSupported = checkAccelerator();
}

bool HelperActions::checkTouchPad()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.deviceexists");
    action.setHelperID(HELPER_ID);
    action.addArgument("device", "touchpad");
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kDebug() << reply.errorDescription() << "(" << reply.errorCode() << ")";

        return false;
    }

    return true;
}

bool HelperActions::checkIllumination()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.deviceexists");
    action.setHelperID(HELPER_ID);
    action.addArgument("device", "illumination");
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kDebug() << reply.errorDescription() << "(" << reply.errorCode() << ")";

        return false;
    }

    return true;
}

bool HelperActions::checkECO()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.deviceexists");
    action.setHelperID(HELPER_ID);
    action.addArgument("device", "eco_mode");
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kDebug() << reply.errorDescription() << "(" << reply.errorCode() << ")";

        return false;
    }

    return true;
}

bool HelperActions::checkKBDBacklight()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.deviceexists");
    action.setHelperID(HELPER_ID);
    action.addArgument("device", "kbd_backlight_mode");
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kDebug() << reply.errorDescription() << "(" << reply.errorCode() << ")";

        return false;
    }

    return true;
}

bool HelperActions::checkAccelerator()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.deviceexists");
    action.setHelperID(HELPER_ID);
    action.addArgument("device", "accelerator");
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kDebug() << reply.errorDescription() << "(" << reply.errorCode() << ")";

        return false;
    }

    return true;
}

void HelperActions::toggleTouchPad()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.toggletouchpad");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.toggletouchpad failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

bool HelperActions::getIllumination()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.illumination");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setillumination failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }

    return (reply.data()["state"].toInt()) ? true : false;
}

void HelperActions::setIllumination(bool enabled)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setillumination");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", (enabled ? 1 : 0));
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setillumination failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

bool HelperActions::getEcoLed()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.eco");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.seteco failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }

    return (reply.data()["state"].toInt()) ? true : false;
}

void HelperActions::setEcoLed(bool enabled)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.seteco");
    action.setHelperID(HELPER_ID);
    action.addArgument("state", (enabled ? 1 : 0));
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.seteco failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getKBDMode()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.kbdmode");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.kbdmode failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }

    return reply.data()["mode"].toInt();
}

void HelperActions::setKBDMode(int mode)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdmode");
    action.setHelperID(HELPER_ID);
    action.addArgument("mode", mode);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdmode failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }

    emit kbdModeChanged(mode);
}

int HelperActions::getKBDTimeout()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.kbdtimeout");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.kbdtimeout failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }

    return reply.data()["time"].toInt();
}

void HelperActions::setKBDTimeout(int time)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setkbdtimeout");
    action.setHelperID(HELPER_ID);
    action.addArgument("time", time);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setkbdtimeout failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

int HelperActions::getProtectionLevel()
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.protectionlevel");
    action.setHelperID(HELPER_ID);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.protectionlevel failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }

    return reply.data()["level"].toInt();
}

void HelperActions::setProtectionLevel(int level)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.setprotectionlevel");
    action.setHelperID(HELPER_ID);
    action.addArgument("level", level);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.setprotectionlevel failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}

void HelperActions::unloadHeads(int timeout)
{
    KAuth::Action action("net.sourceforge.ktoshiba.ktoshhelper.unloadheads");
    action.setHelperID(HELPER_ID);
    action.addArgument("timeout", timeout);
    KAuth::ActionReply reply = action.execute();
    if (reply.failed()) {
        kError() << "net.sourceforge.ktoshiba.ktoshhelper.unloadheads failed" << endl
                 << reply.errorDescription() << "(" << reply.errorCode() << ")";
    }
}


#include "helperactions.moc"

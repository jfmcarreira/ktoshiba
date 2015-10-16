/*
   Copyright (C) 2015 Azael Avalos <coproscefalo@gmail.com>

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

extern "C" {
#include <linux/toshiba.h>
}

#include "generalsettings.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

GeneralSettings::GeneralSettings(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings *>(QObject::parent()))
{
    setupUi(this);

    m_touchpadSupported = isTouchPadSupported();
    m_rapidchargeSupported = isRapidChargeSupported();
    m_usbthreeSupported = isUSBThreeSupported();
}

bool GeneralSettings::isTouchPadSupported()
{
    m_touchpad = m_sys->hw()->getTouchPad();

    if (m_touchpad != 0 && m_touchpad != 1)
        return false;

    return true;
}

bool GeneralSettings::isRapidChargeSupported()
{
    m_rapidcharge = m_sys->hw()->getUSBRapidCharge();

    if (m_rapidcharge != 0 && m_rapidcharge != 1)
        return false;

    return true;
}

bool GeneralSettings::isUSBThreeSupported()
{
    m_usbthree = m_sys->hw()->getUSBThree();

    if (m_usbthree != 0 && m_usbthree != 1)
        return false;

    return true;
}

void GeneralSettings::load()
{
    // TouchPad
    if (m_touchpadSupported)
        touchpad_checkbox->setChecked(m_touchpad ? true : false);
    else
        touchpad_checkbox->setEnabled(false);
    // USB Rapid Charge
    if (m_rapidchargeSupported)
        rapid_charge_checkbox->setChecked(m_rapidcharge ? true : false);
    else
        rapid_charge_checkbox->setEnabled(false);
    // USB Three
    if (m_usbthreeSupported)
        usb_three_checkbox->setChecked(m_usbthree ? true : false);
    else
        usb_three_checkbox->setEnabled(false);
}

void GeneralSettings::save()
{
    unsigned int tmp;

    // TouchPad
    if (m_touchpadSupported) {
        tmp = touchpad_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_touchpad != tmp) {
            m_sys->hw()->setTouchPad(tmp);
            m_touchpad = tmp;
        }
    }
    // USB Rapid Charge
    if (m_rapidchargeSupported) {
        tmp = rapid_charge_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_rapidcharge != tmp) {
            m_sys->hw()->setUSBRapidCharge(tmp);
            m_rapidcharge = tmp;
        }
    }
    // USB Three
    if (m_usbthreeSupported) {
        tmp = usb_three_checkbox->checkState() == Qt::Checked ? 1 : 0;
        if (m_usbthree != tmp) {
            m_sys->hw()->setUSBThree(tmp);
            m_usbthree = tmp;
        }
    }
}

void GeneralSettings::defaults()
{
    // TouchPad
    if (m_touchpadSupported && !m_touchpad)
        touchpad_checkbox->setChecked(true);
    // USB Rapid Charge
    if (m_rapidchargeSupported && m_rapidcharge)
        rapid_charge_checkbox->setChecked(false);
    // USB Three
    if (m_usbthreeSupported && !m_usbthree)
        usb_three_checkbox->setChecked(true);
}

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

#include "generalsettings.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

GeneralSettings::GeneralSettings(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings *>(QObject::parent()))
{
    setupUi(this);

    m_touchpadSupported = isTouchPadSupported();
    m_rapidChargeSupported = isRapidChargeSupported();
    m_usbThreeSupported = isUSBThreeSupported();
}

bool GeneralSettings::isTouchPadSupported()
{
    m_touchpad = m_sys->hw()->getTouchPad();

    if (m_touchpad != KToshibaHardware::TCI_DISABLED && m_touchpad != KToshibaHardware::TCI_ENABLED)
        return false;

    return true;
}

bool GeneralSettings::isRapidChargeSupported()
{
    m_rapidCharge = m_sys->hw()->getUSBRapidCharge();

    if (m_rapidCharge != KToshibaHardware::TCI_DISABLED && m_rapidCharge != KToshibaHardware::TCI_ENABLED)
        return false;

    return true;
}

bool GeneralSettings::isUSBThreeSupported()
{
    m_usbThree = m_sys->hw()->getUSBThree();

    if (m_usbThree != KToshibaHardware::TCI_DISABLED && m_usbThree != KToshibaHardware::TCI_ENABLED)
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
    if (m_rapidChargeSupported)
        rapid_charge_checkbox->setChecked(m_rapidCharge ? true : false);
    else
        rapid_charge_checkbox->setEnabled(false);
    // USB Three
    if (m_usbThreeSupported)
        usb_three_checkbox->setChecked(m_usbThree ? true : false);
    else
        usb_three_checkbox->setEnabled(false);
}

void GeneralSettings::save()
{
    unsigned int tmp;

    // TouchPad
    if (m_touchpadSupported) {
        tmp = (touchpad_checkbox->checkState() == Qt::Checked) ?
                KToshibaHardware::TCI_ENABLED : KToshibaHardware::TCI_DISABLED;
        if (m_touchpad != tmp) {
            m_sys->hw()->setTouchPad(tmp);
            m_touchpad = tmp;
        }
    }
    // USB Rapid Charge
    if (m_rapidChargeSupported) {
        tmp = (rapid_charge_checkbox->checkState() == Qt::Checked) ?
                KToshibaHardware::TCI_ENABLED : KToshibaHardware::TCI_DISABLED;
        if (m_rapidCharge != tmp) {
            m_sys->hw()->setUSBRapidCharge(tmp);
            m_rapidCharge = tmp;
        }
    }
    // USB Three
    if (m_usbThreeSupported) {
        tmp = (usb_three_checkbox->checkState() == Qt::Checked) ?
                KToshibaHardware::TCI_ENABLED : KToshibaHardware::TCI_DISABLED;
        if (m_usbThree != tmp) {
            m_sys->hw()->setUSBThree(tmp);
            m_usbThree = tmp;
        }
    }
}

void GeneralSettings::defaults()
{
    // TouchPad
    if (m_touchpadSupported && !m_touchpad)
        touchpad_checkbox->setChecked(true);
    // USB Rapid Charge
    if (m_rapidChargeSupported && m_rapidCharge)
        rapid_charge_checkbox->setChecked(false);
    // USB Three
    if (m_usbThreeSupported && !m_usbThree)
        usb_three_checkbox->setChecked(true);
}

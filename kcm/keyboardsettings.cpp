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

#include <KLocalizedString>

#include "keyboardsettings.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

KeyboardSettings::KeyboardSettings(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings * >(QObject::parent()))
{
    setupUi(this);

    m_keyboardModesGen1 << "FN-Z" << "AUTO";
    m_keyboardModesGen2 << i18n("TIMER") << i18n("ON") << i18n("OFF");

    m_keyboardFunctionsSupported = isKeyboardFunctionsSupported();
    m_kbdBacklightSupported = isKeyboardBacklightSupported();
}

bool KeyboardSettings::isKeyboardBacklightSupported()
{
    quint32 result = m_sys->hw()->getKBDBacklight(&m_keyboardMode, &m_keyboardTime, &m_keyboardType);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

bool KeyboardSettings::isKeyboardFunctionsSupported()
{
    m_keyboardFunctions = m_sys->hw()->getKBDFunctions();

    if (m_keyboardFunctions != KToshibaHardware::DEACTIVATED
        && m_keyboardFunctions != KToshibaHardware::ACTIVATED)
        return false;

    return true;
}

void KeyboardSettings::load()
{
    // Keyboard Functions
    if (m_keyboardFunctionsSupported) {
        kbd_functions_combobox->setCurrentIndex(m_keyboardFunctions);
        m_keyboardFunctionsToolTip = i18n("Select how the keyboard Function keys operate.<br/>"
                                          "Normal: The F{1-12} keys are as usual and the hotkeys are accessed via FN-F{1-12}.<br/>"
                                          "Function: The F{1-12} keys trigger the hotkey and the F{1-12} keys are accessed via FN-F{1-12}.");
        kbd_functions_combobox->setToolTip(m_keyboardFunctionsToolTip);
        kbd_functions_combobox->setWhatsThis(m_keyboardFunctionsToolTip);
    } else {
        function_keys_label->setEnabled(false);
        kbd_functions_combobox->setEnabled(false);
    }
    // Keyboard Backlight
    if (m_kbdBacklightSupported) {
        m_keyboardIndex = 0;
        if (m_keyboardType == FirstKeyboardGen) {
            kbd_backlight_combobox->addItems(m_keyboardModesGen1);
            m_keyboardIndex = m_keyboardMode == KToshibaHardware::FNZ ? FNZ : AUTO;
            m_tooltip = i18n("Select the keyboard backlight operation mode.<br/>"
                             "FN-Z: User toggles the keyboard backlight.<br/>"
                             "AUTO: Keyboard backlight turns on/off automatically.");
        } else if (m_keyboardType == SecondKeyboardGen) {
            kbd_backlight_combobox->addItems(m_keyboardModesGen2);
            if (m_keyboardMode == KToshibaHardware::TIMER)
                m_keyboardIndex = TIMER;
            else if (m_keyboardMode == KToshibaHardware::ON)
                m_keyboardIndex = ON;
            else if (m_keyboardMode == KToshibaHardware::OFF)
                m_keyboardIndex = OFF;
            m_tooltip = i18n("Select the keyboard backlight operation mode.<br/>"
                             "TIMER: Keyboard backlight turns on/off automatically.<br/>"
                             "ON: Keyboard backlight is turned on.<br/>"
                             "OFF: Keyboard backlight is turned off.");
        }
        kbd_backlight_combobox->setCurrentIndex(m_keyboardIndex);
        kbd_backlight_combobox->setToolTip(m_tooltip);
        kbd_backlight_combobox->setWhatsThis(m_tooltip);

        if (m_keyboardMode == KToshibaHardware::TIMER) {
            kbd_timeout->setText(QString::number(m_keyboardTime) + i18n(" sec"));
            kbd_timeout_slider->setValue(m_keyboardTime);
        } else {
            kbd_timeout_label->setEnabled(false);
            kbd_timeout_slider->setEnabled(false);
        }
    } else {
        groupBox->setEnabled(false);
    }
}

void KeyboardSettings::save()
{
    int tmp;

    // Keyboard Functions
    if (m_keyboardFunctionsSupported) {
        tmp = kbd_functions_combobox->currentIndex();
        if (m_keyboardFunctions != tmp) {
            m_sys->hw()->setKBDFunctions(tmp);
            m_keyboardFunctions = tmp;
        }
    }
    // Keyboard Backlight
    if (m_kbdBacklightSupported) {
        tmp = kbd_backlight_combobox->currentIndex();
        int mode;
        if (m_keyboardIndex != tmp) {
            // 1st gen. backlit keyboards
            if (m_keyboardType == FirstKeyboardGen) {
                mode = (tmp == FNZ) ? KToshibaHardware::FNZ : KToshibaHardware::TIMER;
                // 2nd gen. backlit keyboards
            } else if (m_keyboardType == SecondKeyboardGen && tmp == TIMER) {
                mode = KToshibaHardware::TIMER;
                kbd_timeout_label->setEnabled(true);
                kbd_timeout_slider->setEnabled(true);
                kbd_timeout->setText(QString::number(m_keyboardTime) + i18n(" sec"));
                kbd_timeout_slider->setValue(m_keyboardTime);
            } else if (m_keyboardType == SecondKeyboardGen && tmp == ON) {
                mode = KToshibaHardware::ON;
                kbd_timeout_label->setEnabled(false);
                kbd_timeout_slider->setEnabled(false);
                kbd_timeout->setEnabled(false);
            } else if (m_keyboardType == SecondKeyboardGen && tmp == OFF) {
                mode = KToshibaHardware::OFF;
                kbd_timeout_label->setEnabled(false);
                kbd_timeout_slider->setEnabled(false);
                kbd_timeout->setEnabled(false);
            }
            m_sys->hw()->setKBDBacklight(m_keyboardType == FirstKeyboardGen ? m_keyboardMode : mode, m_keyboardTime);
            m_keyboardIndex = tmp;
        }
        if (m_keyboardType == SecondKeyboardGen && m_keyboardMode == KToshibaHardware::TIMER) {
            tmp = kbd_timeout_slider->value();
            if (m_keyboardTime != tmp) {
                m_sys->hw()->setKBDBacklight(mode, tmp);
                m_keyboardTime = tmp;
            }
        }
    }
}

void KeyboardSettings::defaults()
{
    // Keyboard Functions
    if (m_keyboardFunctionsSupported && !m_keyboardFunctions)
        kbd_functions_combobox->setCurrentIndex(1);
    // Keyboard Backlight
    if (m_kbdBacklightSupported) {
        if (m_keyboardMode != KToshibaHardware::TIMER)
            kbd_backlight_combobox->setCurrentIndex(m_keyboardType == FirstKeyboardGen ? AUTO : TIMER);
        if (m_keyboardTime != 15)
            kbd_timeout_slider->setValue(15);
    }
}

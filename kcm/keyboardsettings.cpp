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

extern "C" {
#include <linux/toshiba.h>
}

#include "keyboardsettings.h"
#include "systemsettings.h"
#include "ktoshibahardware.h"

KeyboardSettings::KeyboardSettings(QWidget *parent)
    : QWidget(parent),
      m_sys(qobject_cast<KToshibaSystemSettings *>(QObject::parent()))
{
    setupUi(this);

    m_type1 << "FN-Z" << "AUTO";
    m_type2 << i18n("TIMER") << i18n("ON") << i18n("OFF");

    m_kbdFunctionsSupported = isKeyboardFunctionsSupported();
    m_kbdBacklightSupported = isKeyboardBacklightSupported();
}

bool KeyboardSettings::isKeyboardBacklightSupported()
{
    quint32 result = m_sys->hw()->getKBDBacklight(&m_mode, &m_time, &m_type);

    if (result != KToshibaHardware::SUCCESS && result != KToshibaHardware::SUCCESS2)
        return false;

    return true;
}

bool KeyboardSettings::isKeyboardFunctionsSupported()
{
    m_kbdfunctions = m_sys->hw()->getKBDFunctions();

    if (m_kbdfunctions != 0 && m_kbdfunctions != 1)
        return false;

    return true;
}

void KeyboardSettings::load()
{
    // Keyboard Functions
    if (m_kbdFunctionsSupported) {
        kbd_functions_combobox->setCurrentIndex(m_kbdfunctions);
        m_functionsToolTip = i18n("Select how the keyboard Function keys operate.<br/>"
                                  "Normal: The F{1-12} keys are as usual and the hotkeys are accessed via FN-F{1-12}.<br/>"
                                  "Function: The F{1-12} keys trigger the hotkey and the F{1-12} keys are accessed via FN-F{1-12}.");
        kbd_functions_combobox->setToolTip(m_functionsToolTip);
        kbd_functions_combobox->setWhatsThis(m_functionsToolTip);
    } else {
        function_keys_label->setEnabled(false);
        kbd_functions_combobox->setEnabled(false);
    }
    // Keyboard Backlight
    if (m_kbdBacklightSupported) {
        m_index = 0;
        if (m_type == 1) {
            kbd_backlight_combobox->addItems(m_type1);
            m_index = m_mode == 1 ? 0 : 1;
            m_tooltip = i18n("Select the keyboard backlight operation mode.<br/>"
                             "FN-Z: User toggles the keyboard backlight.<br/>"
                             "AUTO: Keyboard backlight turns on/off automatically.");
        } else if (m_type == 2) {
            kbd_backlight_combobox->addItems(m_type2);
            if (m_mode == 2)
                m_index = 0;
            else if (m_mode == 8)
                m_index = 1;
            else if (m_mode == 16)
                m_index = 2;
            m_tooltip = i18n("Select the keyboard backlight operation mode.<br/>"
                             "TIMER: Keyboard backlight turns on/off automatically.<br/>"
                             "ON: Keyboard backlight is turned on.<br/>"
                             "OFF: Keyboard backlight is turned off.");
        }
        kbd_backlight_combobox->setCurrentIndex(m_index);
        kbd_backlight_combobox->setToolTip(m_tooltip);
        kbd_backlight_combobox->setWhatsThis(m_tooltip);

        if (m_mode == 2) {
            kbd_timeout->setText(QString::number(m_time) + i18n(" sec"));
            kbd_timeout_slider->setValue(m_time);
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
    if (m_kbdFunctionsSupported) {
        tmp = kbd_functions_combobox->currentIndex();
        if (m_kbdfunctions != tmp) {
            m_sys->hw()->setKBDFunctions(tmp);
            m_kbdfunctions = tmp;
        }
    }
    // Keyboard Backlight
    if (m_kbdBacklightSupported) {
        tmp = kbd_backlight_combobox->currentIndex();
        int mode;
        if (m_index != tmp) {
            // 1st gen. backlit keyboards
            if (m_type == 1) {
                mode = (tmp == 0) ? KToshibaHardware::FNZ : KToshibaHardware::TIMER;
            // 2nd gen. backlit keyboards
            } else if (m_type == 2 && tmp == 0) {
                mode = KToshibaHardware::TIMER;
                kbd_timeout_label->setEnabled(true);
                kbd_timeout_slider->setEnabled(true);
                kbd_timeout->setText(QString::number(m_time) + i18n(" sec"));
                kbd_timeout_slider->setValue(m_time);
            } else if (m_type == 2 && tmp == 1) {
                mode = KToshibaHardware::ON;
                kbd_timeout_label->setEnabled(false);
                kbd_timeout_slider->setEnabled(false);
                kbd_timeout->setEnabled(false);
            } else if (m_type == 2 && tmp == 2) {
                mode = KToshibaHardware::OFF;
                kbd_timeout_label->setEnabled(false);
                kbd_timeout_slider->setEnabled(false);
                kbd_timeout->setEnabled(false);
            }
            m_sys->hw()->setKBDBacklight(m_type == 1 ? m_mode : mode, m_time);
            m_index = tmp;
        }
        if (m_type == 2 && m_mode == KToshibaHardware::TIMER) {
            tmp = kbd_timeout_slider->value();
            if (m_time != tmp) {
                m_sys->hw()->setKBDBacklight(mode, tmp);
                m_time = tmp;
            }
        }
    }
}

void KeyboardSettings::defaults()
{
    // Keyboard Functions
    if (m_kbdFunctionsSupported && !m_kbdfunctions)
        kbd_functions_combobox->setCurrentIndex(1);
    // Keyboard Backlight
    if (m_kbdBacklightSupported) {
        if (m_mode != KToshibaHardware::TIMER)
            kbd_backlight_combobox->setCurrentIndex(m_type == 1 ? 1 : 0);
        if (m_time != 15)
            kbd_timeout_slider->setValue(15);
    }
}

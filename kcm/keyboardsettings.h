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

#ifndef KEYBOARDSETTINGS_H
#define KEYBOARDSETTINGS_H

#include "ui_keyboard.h"

class KToshibaSystemSettings;

class KeyboardSettings : public QWidget, public Ui::Keyboard
{
    Q_OBJECT

public:
    explicit KeyboardSettings(QWidget *parent = 0);

    void load();
    void save();
    void defaults();

    int m_type;

private:
    bool isKeyboardFunctionsSupported();
    bool isKeyboardBacklightSupported();

    KToshibaSystemSettings *m_sys;

    bool m_kbdFunctionsSupported;
    QString m_functionsToolTip;
    int m_kbdfunctions;

    bool m_kbdBacklightSupported;
    QStringList m_type1;
    QStringList m_type2;
    QString m_tooltip;
    int m_mode;
    int m_index;
    int m_time;
};

#endif // KEYBOARDSETTINGS_H

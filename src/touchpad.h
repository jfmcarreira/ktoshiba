 /*
   Copyright (C) 2004-2009  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef TOUCHPAD_H
#define TOUCHPAD_H

extern "C" {
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>
}

class TouchPad
{
public:
    enum TouchPadError { NoError, NoX, NoXInput, NoExtension, NoTouchPad };
    enum TouchPadState { Off = 0, On = 1, TapToClickOff = 2 };

public:
    TouchPad();
    ~TouchPad();

    int init();
    void setTouchPad(int state);
    int getTouchPad();

    int m_touchpad;

private:
    void clean();
    int check_XI_version();
    int findDeviceID();
    void setProperty(int);
    int getProperty();

    Display *m_Display;
    XDevice *m_Device;
    XDeviceInfo *m_DeviceInfo;
    Atom *m_Properties;
    Atom m_TouchPadOffProperty;
    XID m_DeviceID;
};

#endif // TOUCHPAD_H

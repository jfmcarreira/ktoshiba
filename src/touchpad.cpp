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

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/extensions/XI.h>
}

#include "touchpad.h"

TouchPad::TouchPad()
    : m_Display( NULL )
{
}

TouchPad::~TouchPad()
{
    clean();
}

int TouchPad::init()
{
    int xi_opcode, event, error;

    m_Display = XOpenDisplay(NULL);

    if (m_Display == NULL) {
        fprintf(stderr, "Unable to connect to X server\n");
        return TouchPad::NoX;
    }

    if (!XQueryExtension(m_Display, "XInputExtension", &xi_opcode, &event, &error)) {
        fprintf(stderr, "X Input extension not available.\n");
        return TouchPad::NoXInput;
    }

    if (!check_XI_version()) {
        fprintf(stderr, "%s extension not available.\n", INAME);
        return TouchPad::NoExtension;
    }

    findDeviceID();

    return TouchPad::NoError;
}

void TouchPad::clean()
{
    if (m_Display != NULL) {
        XSync(m_Display, False);
        XCloseDisplay(m_Display);
        m_Display = NULL;
    }
}

int TouchPad::check_XI_version()
{
    XExtensionVersion *version;
    int ver = -1;

    version = XGetExtensionVersion(m_Display, INAME);

    if (version && (version != (XExtensionVersion*) NoSuchExtension)) {
        ver = version->major_version;
        XFree(version);
    }

    return ver;
}

void TouchPad::findDeviceID()
{
    int devices;

    m_DeviceInfo = XListInputDevices(m_Display, &devices);
    if (!m_DeviceInfo) {
        fprintf(stderr, "Could not get input devices.\n");
        return;
    }

    for (int i = 0; i < devices; i++) {
        if (strcmp(m_DeviceInfo[i].name, XI_TOUCHPAD)) {
            m_DeviceID = m_DeviceInfo[i].id;
        }
    }

    XFreeDeviceList(m_DeviceInfo);
}

int TouchPad::getProperty()
{
    int state = -1;
    int properties, actual_format, size;
    char *name;
    unsigned long num_items, bytes_after;
    unsigned char *data;
    Atom actual_type;

    m_Device = XOpenDevice(m_Display, m_DeviceID);
    if (!m_Device) {
        fprintf(stderr, "Could not open %s.\n", XI_TOUCHPAD);
        return -1;
    }

    m_Properties = XListDeviceProperties(m_Display, m_Device, &properties);
    if (!properties) {
        // Nothing left to do...
        fprintf(stderr, "The %s does not have any properties...\n", XI_TOUCHPAD);
        XFree(m_Properties);
        XCloseDevice(m_Display, m_Device);
        return -1;
    }

    // Let's find "Synaptics Off" property
    while (properties--) {
        name = XGetAtomName(m_Display, m_Properties[properties]);
        if (!strcmp(name, "Synaptics Off")) {
            // And stop looking once found...
            m_TouchPadOffProperty = m_Properties[properties];

            if (XGetDeviceProperty(m_Display, m_Device, m_TouchPadOffProperty, 0, 1000, False,
                                   AnyPropertyType, &actual_type, &actual_format, &num_items,
                                   &bytes_after, &data) == Success) {
                if (num_items == 0) {
                    // Nothing left to do here either...
                    fprintf(stderr, "The %s does not have any properties...\n", XI_TOUCHPAD);
                    XFree(data);
                    XFree(m_Properties);
                    XCloseDevice(m_Display, m_Device);
                    return -1;
                }

                size = sizeof(char);
                state = (*((char*)data));
            }
            break;
        }
    }
    XFree(data);
    XFree(m_Properties);
    XCloseDevice(m_Display, m_Device);

    return state;
}

void TouchPad::setProperty(int state)
{
    int properties, format = 8, elements = 1;
    char *name, *data;
    Atom property;

    m_Device = XOpenDevice(m_Display, m_DeviceID);
    if (!m_Device) {
        fprintf(stderr, "Could not open %s.\n", XI_TOUCHPAD);
        return;
    }

    m_Properties = XListDeviceProperties(m_Display, m_Device, &properties);
    if (!properties) {
        // Nothing left to do...
        fprintf(stderr, "The %s does not have any properties...\n", XI_TOUCHPAD);
        XFree(m_Properties);
        XCloseDevice(m_Display, m_Device);
        return;
    }

    // Let's find "Synaptics Off" property
    while (properties--) {
        name = XGetAtomName(m_Display, m_Properties[properties]);
        if (!strcmp(name, "Synaptics Off")) {
            // And stop looking once found...
            m_TouchPadOffProperty = m_Properties[properties];
            break;
        }
    }

    property = XInternAtom(m_Display, name, True);
    data = (char*)calloc(elements, sizeof(long));
    (*((char*)data)) = state;
    
    XChangeDeviceProperty(m_Display, m_Device, property, XA_INTEGER, format,
                          PropModeReplace, (unsigned char*)data, elements);

    free(data);
    XFree(m_Properties);
    XCloseDevice(m_Display, m_Device);
}

int TouchPad::getTouchPad()
{
    return getProperty();
}

void TouchPad::setTouchPad(int state)
{
    if (state == TouchPad::On || state == TouchPad::Off)
        setProperty(state);
}


#include "touchpad.h"

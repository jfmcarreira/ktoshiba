/***************************************************************************
 *   Copyright (C) 2006 by Azael Avalos                                    *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   Based on Lineakd by Sheldon Lee Wen <leewsb@hotmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <kdebug.h>
#include <dcopclient.h>

extern "C" {
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XKBfile.h>

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
}

// Global variables
Display *mDisplay;
Window mWindow;
XkbFileInfo result;
pid_t pid;

static void signal_handler(int signal)
{
    kdDebug() << "ktosh_keyhandler: Received signal " << signal << ". Exiting..." << endl;

    if (result.xkb != NULL)
        XkbFreeClientMap(result.xkb, XkbAllMapComponentsMask, True);
    if (mDisplay != NULL) {
        XCloseDisplay(mDisplay);
        mDisplay = NULL;
    }

    exit(0);
}

static int XErrHandler(Display *display, XErrorEvent *XErrEv)
{
    if (display != NULL) ;

    static int ret = (int)(XErrEv->request_code);
    kdError() << "ktosh_keyhandler: *** Xlib error caught ***" << endl;
    kdError() << "Major opcode of failed request: " << (int)(XErrEv->request_code) << " (XKEYBOARD)" << endl;
    kdError() << "Minor opcode of failed request: " << (int)(XErrEv->minor_code) << " (XkbSetMap)" << endl;
    kdError() << "Resource ID of failed request: " << XErrEv->resourceid << endl;
    kdError() << "Serial number of failed request: " << XErrEv->serial << endl;
    kdError() << "Error code: " << (int)(XErrEv->error_code) << endl;
    kdError() << "Type: " << XErrEv->type << endl;

    return ret;
}

void grabKeys()
{
    XGrabKey(mDisplay, 144, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Previous
    XGrabKey(mDisplay, 145, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Fn-1 (Volume Down)
    XGrabKey(mDisplay, 146, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Fn-2 (Volume Up)
    XGrabKey(mDisplay, 147, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Media Player
    XGrabKey(mDisplay, 148, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Toggle Mode
    XGrabKey(mDisplay, 149, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Battery Status
    XGrabKey(mDisplay, 150, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Fn-F1
    XGrabKey(mDisplay, 151, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Fn-F9 (MousePad On)
    XGrabKey(mDisplay, 152, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Fn-F9 (MousePad Off)
    XGrabKey(mDisplay, 153, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Play/Pause
    XGrabKey(mDisplay, 159, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Fn-F7
    XGrabKey(mDisplay, 160, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Fn-Esc
    XGrabKey(mDisplay, 161, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Fn-F8 (Wireless On/Off)
    XGrabKey(mDisplay, 162, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Next
    XGrabKey(mDisplay, 163, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Bluetooth On/Off
    XGrabKey(mDisplay, 164, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Stop/Eject
    XGrabKey(mDisplay, 178, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// WWW
    XGrabKey(mDisplay, 236, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Console Direct Access
    XGrabKey(mDisplay, 239, AnyModifier, mWindow, False, GrabModeAsync, GrabModeAsync);	// Fn-F6
}

int main(void)
{
    XEvent event;
    XModifierKeymap *modmap;
    QByteArray data;
    QDataStream arg(data, IO_WriteOnly);
    int major, minor, error, min_keycodes, max_keycodes;
    int mScreen, mEventBaseRet, mOpcodeRet;
    char *mDisplayName;

    // Signal handling for a _clean_ exit
    signal(SIGTERM, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);

    // Connect to DCOP
    DCOPClient mClient;
    QCString appID = mClient.registerAs("ktosh_keyhandler", false);
    if (mClient.isRegistered())
        kdDebug() << "ktosh_keyhandler: Registered with DCOP as: " << appID << endl;
    else {
        kdError() << "ktosh_keyhandler: Could not register with DCOP server" << endl;
        return -1;
    }

    // Initialize variables
    mDisplay = NULL;
    pid = 0;
    major = XkbMajorVersion;
    minor = XkbMinorVersion;
    mDisplayName = getenv("DISPLAY");
    if (mDisplayName == NULL)
        mDisplayName = ":0.0";
    mScreen = 0;
    mEventBaseRet = 0;
    mOpcodeRet = 0;
    min_keycodes = 1;
    max_keycodes = 17;

    // Open X server connection
    mDisplay = XOpenDisplay(mDisplayName);
    mScreen = DefaultScreen(mDisplay);
    if (mDisplay == NULL) {
        kdError() << "ktosh_keyhandler: Could not connect to X server" << endl;
        return -1;
    } else
    if (mDisplay != NULL) {
        mWindow = DefaultRootWindow(mDisplay);
        XSetErrorHandler(&XErrHandler);
        XDisplayKeycodes(mDisplay, &min_keycodes, &max_keycodes);
        modmap = XGetModifierMapping(mDisplay);
        if (modmap)
            XFreeModifiermap(modmap);
    }

    // Open a keyboard connection
    Display *mKbdDisplay;
    mKbdDisplay = XkbOpenDisplay(mDisplayName, &mEventBaseRet, NULL, &major, &minor, &error);
    if (mKbdDisplay == NULL) {
        kdError() << "ktosh_keyhandler: Could not connect to X (Xkb) server" << endl;
        switch (error) {
            case XkbOD_BadLibraryVersion:
                kdError() << "Bad Library Version" << endl;
                break;
            case XkbOD_ConnectionRefused:
                kdError() << "Connection Refused" << endl;
                break;
            case XkbOD_NonXkbServer:
                kdError() << "Non Xkb Server" << endl;
                break;
            case XkbOD_BadServerVersion:
                kdError() << "Bad Server Version" << endl;
                break;
            default:
                kdError() << "Unknown error " << error << " from XkbOpenDisplay" << endl;
        }
        if (mDisplay != NULL) {
            XCloseDisplay(mDisplay);
            mDisplay = NULL;
            return -1;
        }
    } else
    if (!XkbQueryExtension(mKbdDisplay, &mOpcodeRet, &mEventBaseRet, NULL, &major, &minor))
        kdError() << "ktosh_keyhandler: Cannot initialize the Xkb extension" << endl;

    result.xkb = XkbGetMap(mDisplay, XkbAllMapComponentsMask, XkbUseCoreKbd);
    if (result.xkb == NULL)
        kdError() << "ktosh_keyhandler: Cannot load keyboard description" << endl;

    // Initialize X Kbd
    XAllowEvents(mDisplay, AsyncKeyboard, CurrentTime);
    XSelectInput(mDisplay, mWindow, KeyPressMask | ButtonPressMask);

    // Key grabbing
    grabKeys();

    // And so the _forever_ loop
    while (true) {
        XNextEvent(mDisplay, &event);

        switch (event.type) {
            case KeyPress:
                kdDebug() << "ktosh_keyhandler: Key pressed " << (int)event.xkey.keycode << endl;
                arg << event.xkey.keycode;
                mClient.send("ktoshiba", "actions", "hotkey(int)", data);
                break;
            // For the moment we only care about a key press
            //case KeyRelease:
                //kdDebug() << "ktosh_keyhandler: Key released " << (int)event.xkey.keycode << endl;
                //arg << event.xkey.keycode;
                //mClient.send("ktoshiba", "actions", "hotkey(int)", data);
                //break;
        }

        usleep(100000);
    }

    // We are not supposed to get here...
    return 0;
}

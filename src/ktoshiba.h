/*
   Copyright (C) 2004-2015  Azael Avalos <coproscefalo@gmail.com>

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

#ifndef KTOSHIBA_H
#define KTOSHIBA_H

#include <KStatusNotifierItem>

class QAction;
class QMenu;

class FnActions;

/**
 * @short Hotkeys monitoring for Toshiba laptops
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 5.1
 */
class KToshiba : public KStatusNotifierItem
{
    Q_OBJECT

public:
    KToshiba();
    virtual ~KToshiba();

    bool initialize();

private Q_SLOTS:
    void notifyHDDMovement();
    void configureClicked();

private:
    void cleanup();

    FnActions *m_fn;

    QAction *m_configure;
    QMenu *m_popupMenu;

};

#endif // KTOSHIBA_H

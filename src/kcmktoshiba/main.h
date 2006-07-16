/***************************************************************************
 *   main.h                                                                *
 *                                                                         *
 *   Copyright (C) 2004-2006 by Azael Avalos                               *
 *   coproscefalo@gmail.com                                                *
 *                                                                         *
 *   Based on kcm_kvaio                                                    *
 *   Copyright (C) 2003 Mirko Boehm (mirko@kde.org)                        *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
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

#ifndef KCMTOSHIBA_MAIN_H
#define KCMTOSHIBA_MAIN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kcmodule.h>

class QTimer;
class KCMKToshibaGeneral;

class KToshibaSMMInterface;
class KToshibaProcInterface;

/**
 * @author Azael Avalos <coproscefalo@gmail.com>
 * @version 0.4
 */
class KCMToshibaModule: public KCModule
{
    Q_OBJECT
public:
    KCMToshibaModule(QWidget *parent=0, const char *name = 0, const QStringList & = QStringList());

    void load();
    void save();
    void defaults();
protected slots:
    void configChanged();
    void timeout();
private:
    KCMKToshibaGeneral *m_KCMKToshibaGeneral;
    KToshibaSMMInterface *m_SMMIFace;
    KToshibaProcInterface *m_ProcIFace;
    QTimer *m_Timer;
    bool m_InterfaceAvailable;
    bool m_Omnibook;
    bool m_Init;
    int m_AC;
};

#endif // KCMTOSHIBA_MAIN_H

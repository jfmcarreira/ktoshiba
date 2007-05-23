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

#include <kcmodule.h>

class QTimer;

class KCMKToshibaGeneral;
class KToshibaProcInterface;
class KToshibaSMMInterface;
class KToshibaOmnibookInterface;
class BSMWidget;

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
    void bsmLoad(int);
    void bsmChanged();
    void timeout();
    void hwChanged();
private:
    KCMKToshibaGeneral *m_KCMKToshibaGeneral;
    KToshibaProcInterface *m_ProcIFace;
    KToshibaSMMInterface *m_SMMIFace;
    KToshibaOmnibookInterface *m_OmniIFace;
    BSMWidget *m_BSMWidget;
    QTimer *m_Timer;
    bool m_Omnibook;
    bool m_SCIFace;
    bool m_HCIFace;
    bool m_Hotkeys;
    bool m_Hardware;
    bool m_Init;
    int m_IFaceErr;
    int m_AC;
    int m_BSM;
};

#endif // KCMTOSHIBA_MAIN_H

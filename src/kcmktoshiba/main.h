/***************************************************************************
 *   main.h                                                                *
 *                                                                         *
 *   Copyright (C) 2004 by Azael Avalos                                    *
 *   neftali@utep.edu                                                      *
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

#include "../ktoshibasmminterface.h"

class QTimer;
class KCMKToshibaGeneral;

/**
 * @author Azael Avalos <neftali@utep.edu>
 * @version 0.1
 */
class KCMToshibaModule: public KCModule
{
    Q_OBJECT
public:
    KCMToshibaModule( QWidget *parent=0, const char *name=0, const QStringList& = QStringList() );

    void load();
    void save();
    void defaults();
    QString quickHelp() const;
public slots:
    void configChanged();
protected slots:
	void timeout();
private:
	KCMKToshibaGeneral *m_KCMKToshibaGeneral;
	KToshibaSMMInterface *m_Driver;
	QTimer *mTimer;
	bool m_InterfaceAvailable;
	int m_AC;
};

#endif // KCMTOSHIBA_MAIN_H

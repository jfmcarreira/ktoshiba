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

#ifndef FN_ACTIONS_H
#define FN_ACTIONS_H

#include <QObject>
#include <QString>

//#include "ui_statuswidget.h"

class KToshibaDBusInterface;

class FnActions : public QObject
{
    Q_OBJECT
    
public:
    FnActions(QObject *parent);
    ~FnActions();

    QString modelName();

private Q_SLOTS:
    void slotGotHotkey(QString);
    
protected:
    //void showWidget();
    //Ui::StatusWidget m_statusWidget;

private:
    //QWidget *widget;
    KToshibaDBusInterface* m_dBus;
    bool m_bright;
};

#endif // FN_ACTIONS_H

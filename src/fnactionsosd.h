/*
 * Copyright (C) 2004-2016  Azael Avalos <coproscefalo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef FN_ACTIONS_OSD_H
#define FN_ACTIONS_OSD_H

#include <QTimer>
#include <QIcon>
#include <QString>

class QWidget;
class QLabel;

class FnActionsOsd: public QWidget
{
public:
    FnActionsOsd(QWidget *parent = NULL);
    void showInfo(const QIcon& icon, const QString& text);

private:
    QTimer *m_widgetTimer;

    int m_iconSize;
    QLabel *m_statusIcon;
    QLabel *m_statusLabel;

};

#endif // FN_ACTIONS_OSD_H

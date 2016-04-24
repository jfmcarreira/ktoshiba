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


#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "fnactionsosd.h"

FnActionsOsd::FnActionsOsd(QWidget *parent)
    : QWidget(parent),
    m_widgetTimer(new QTimer(this))
{

    m_widgetTimer->setSingleShot(true);
    connect(m_widgetTimer, SIGNAL(timeout()), this, SLOT(hide()));

    setObjectName(QStringLiteral("StatusWidget"));
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint
                    | Qt::FramelessWindowHint | Qt::WindowTransparentForInput);
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_iconSize = 120;
    int widgetWidth = m_iconSize + 10;

    QSize widgetSize(widgetWidth, widgetWidth);
    resize(widgetWidth, widgetWidth);
    setMinimumSize(widgetWidth, widgetWidth);
    setMaximumSize(widgetWidth, widgetWidth);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setAlignment(Qt::AlignHCenter);

    m_statusIcon = new QLabel;
    m_statusIcon->setMinimumSize(QSize(widgetWidth, m_iconSize));
    m_statusIcon->setMaximumSize(QSize(widgetWidth, m_iconSize));
    m_statusIcon->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    m_statusLabel = new QLabel;
    m_statusLabel->setMinimumSize(QSize(widgetWidth, 32));
    m_statusLabel->setMaximumSize(QSize(widgetWidth, 32));
    m_statusLabel->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    mainLayout->addWidget(m_statusIcon);
    mainLayout->addWidget(m_statusLabel);
    setLayout(mainLayout);

    m_statusIcon->setText(QString());
    m_statusLabel->setText(QString());

}

void FnActionsOsd::showInfo(const QIcon& icon, const QString& text)
{
    m_statusIcon->setPixmap(icon.pixmap(m_iconSize,m_iconSize));
    m_statusLabel->setText(text);
    m_widgetTimer->start(1500);
}



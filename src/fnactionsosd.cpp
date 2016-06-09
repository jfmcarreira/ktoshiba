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

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QDBusConnection>

#include <KDeclarative/QmlObject>
#include <KPackage/PackageLoader>

#include "fnactionsosd.h"
#include "ktoshiba_debug.h"


#include "osdservice.h"

#define SERVICE QLatin1Literal("org.kde.plasmashell")
#define PATH QLatin1Literal("/org/kde/osdService")
#define CONNECTION QDBusConnection::sessionBus()



FnActionsOsd::FnActionsOsd(QObject *parent)
    : QObject(parent),
    m_widgetTimer(new QTimer(this))
{

    m_widgetTimer->setSingleShot(true);
    m_timeout = 1500;
    connect(m_widgetTimer, SIGNAL(timeout()), this, SLOT(hideOsd()));

    m_osdObject = new KDeclarative::QmlObject(this);
    const QString osdPath = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel")).filePath("osdmainscript");
    m_osdObject->setSource(QUrl::fromLocalFile(osdPath));
    if ( osdPath.isEmpty() || m_osdObject->status() != QQmlComponent::Ready) {
        qCDebug(KTOSHIBA) << "Failed to load the OSD QML file";
        return;
    }
    else {
        qCDebug(KTOSHIBA) << "Loading the OSD QML file from " << osdPath;
    }

    m_timeout = m_osdObject->rootObject()->property("timeout").toInt();

//     QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/kde/osdService"), this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);



}

void FnActionsOsd::showInfo(const QString& icon, const QString& text)
{
    m_widgetTimer->stop();

//     OrgKdeOsdServiceInterface osdService(SERVICE, PATH, CONNECTION);
//     osdService.showText("audio-volume-high", text);
//
//     osdService.keyboardBrightnessChanged(15);

    auto *rootObject = m_osdObject->rootObject();
    if (rootObject) {

        rootObject->setProperty("showingProgress", false);
        rootObject->setProperty("osdValue", text);
        rootObject->setProperty("icon", icon);

        // if our OSD understands animating the opacity, do it;
        // otherwise just show it to not break existing lnf packages
        if (rootObject->property("animateOpacity").isValid()) {
            rootObject->setProperty("animateOpacity", false);
            rootObject->setProperty("opacity", 1);
            rootObject->setProperty("visible", true);
            rootObject->setProperty("animateOpacity", true);
            rootObject->setProperty("opacity", 0);
        } else {
            rootObject->setProperty("visible", true);
        }
    }


    m_widgetTimer->start(m_timeout);
}

void FnActionsOsd::hideOsd()
{
//     hide();

    auto *rootObject = m_osdObject->rootObject();
    if (!rootObject) {
        return;
    }

    rootObject->setProperty("visible", false);

    // this is needed to prevent fading from "old" values when the OSD shows up
    rootObject->setProperty("icon", "");
    rootObject->setProperty("osdValue", 0);
}




/*
   Copyright (C) 2014-2016  Azael Avalos <coproscefalo@gmail.com>

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

#include <QDir>
#include <QSocketNotifier>

extern "C" {
#include <errno.h>
#include <linux/genetlink.h>
}

#include "ktoshibanetlinkevents.h"
#include "ktoshiba_debug.h"

#define HAPS_HID "TOS620A:00"

#define NLMSG_TYPE 0x16

/*
 * The following struct was taken from linux ACPI event.c
 */
struct acpi_genl_event {
    char device_class[20];
    char bus_id[15];
    __u32 type;
    __u32 data;
};

static struct acpi_genl_event *m_event;

static int callback_data(const struct nlmsghdr *nlh, void *data)
{
    Q_UNUSED(data)

    if (nlh->nlmsg_type == NLMSG_ERROR) {
        qCCritical(KTOSHIBA) << "Unknown netlink error received";

        return MNL_CB_ERROR;
    }

		if (nlh->nlmsg_type != NLMSG_TYPE) {
        qCWarning(KTOSHIBA) << "Not an ACPI event message";

        return MNL_CB_ERROR;
    }

    int len = nlh->nlmsg_len;

    len -= NLMSG_LENGTH(GENL_HDRLEN);
    if (len < 0) {
        qCWarning(KTOSHIBA) << "Wrong message len" << len;

        return MNL_CB_ERROR;
    }

    ssize_t offset = GENL_HDRLEN + NLA_HDRLEN;

    m_event = reinterpret_cast<acpi_genl_event *>(mnl_nlmsg_get_payload_offset(nlh, offset));

    qCDebug(KTOSHIBA) << "Class:" << m_event->device_class << "Bus:" << m_event->bus_id
                      << "Type:" << hex << m_event->type << "Data:" << m_event->data;

    return MNL_CB_OK;
}

KToshibaNetlinkEvents::KToshibaNetlinkEvents(QObject *parent)
    : QObject(parent),
      m_nl(NULL),
      m_notifier(NULL)
{
    m_deviceHID = getDeviceHID();
    if (m_deviceHID.isEmpty() || m_deviceHID.isNull()) {
        qCWarning(KTOSHIBA) << "Device HID is not valid, TVAP events monitoring will not be possible";
    }
}

KToshibaNetlinkEvents::~KToshibaNetlinkEvents()
{
    if (m_notifier) {
        m_notifier->setEnabled(false);
        delete m_notifier;
    }

    if (m_nl) {
        mnl_socket_close(m_nl);
    }
}

QString KToshibaNetlinkEvents::getDeviceHID()
{
    m_devices << QStringLiteral("TOS1900:00") << QStringLiteral("TOS6200:00")
              << QStringLiteral("TOS6207:00") << QStringLiteral("TOS6208:00");

    QDir dir;
    QString path(QStringLiteral("/sys/devices/LNXSYSTM:00/LNXSYBUS:00/%1/"));
    Q_FOREACH (const QString &device, m_devices) {
        if (dir.exists(path.arg(device))) {
            return device;
        }
    }

    return QString();
}

bool KToshibaNetlinkEvents::attach()
{
    m_nl = mnl_socket_open(NETLINK_GENERIC);
    if (m_nl == NULL) {
        qCCritical(KTOSHIBA) << "Could not open netlink socket:" << strerror(errno);

        return false;
    }

    int group = 0x2;
    if (mnl_socket_bind(m_nl, group, MNL_SOCKET_AUTOPID) < 0) {
        qCCritical(KTOSHIBA) << "Could not bind to netlink socket:" << strerror(errno);

        return false;
    }

    int socket = mnl_socket_get_fd(m_nl);
    qCDebug(KTOSHIBA) << "Binded to socket" << socket << "for genetlink ACPI events monitoring";

    if (mnl_socket_setsockopt(m_nl, NETLINK_ADD_MEMBERSHIP, &group, sizeof(int)) < 0) {
        qCCritical(KTOSHIBA) << "Could not set socket options:" << strerror(errno);

        return false;
    }

    m_notifier = new QSocketNotifier(socket, QSocketNotifier::Read, this);

    return connect(m_notifier, SIGNAL(activated(int)), this, SLOT(parseEvents(int)));
}

void KToshibaNetlinkEvents::parseEvents(int socket)
{
    if (socket < 0) {
        qCWarning(KTOSHIBA) << "No socket to receive from...";

        return;
    }

    ssize_t len = mnl_socket_recvfrom(m_nl, m_eventBuffer, sizeof(m_eventBuffer));
    if (len <= 0) {
        qCCritical(KTOSHIBA) << "Could not receive netlink data:" << strerror(errno);

        return;
    }

    if (mnl_cb_run(m_eventBuffer, len, 0, 0, callback_data, NULL) != MNL_CB_OK) {
        qCWarning(KTOSHIBA) << "Callback error";

        return;
    }

    QString haps(QStringLiteral(HAPS_HID));
    if (QString::fromUtf8(m_event->bus_id) == QStringLiteral(HAPS_HID)) {
        Q_EMIT hapsEvent(m_event->type);
    }

    if (QString::fromUtf8(m_event->bus_id) == m_deviceHID) {
        Q_EMIT tvapEvent(m_event->type, m_event->data);
    }

    if (QString::fromUtf8(m_event->device_class) == QStringLiteral("ac_adapter")) {
        Q_EMIT acAdapterChanged(m_event->data);
    }
}

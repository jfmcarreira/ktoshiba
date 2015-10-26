/*
   Copyright (C) 2014-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include <QCoreApplication>
#include <QSocketNotifier>
#include <QDebug>

extern "C" {
#include <unistd.h>
#include <errno.h>
}

#include <netlink/genl/genl.h>

#include "ktoshibanetlinkevents.h"

#define TOSHIBA_HAPS   "TOS620A:00"

/*
 * The following enums and defines were taken from
 * linux ACPI event.c
 */
enum {
    ACPI_GENL_ATTR_UNSPEC,
    ACPI_GENL_ATTR_EVENT,
    __ACPI_GENL_ATTR_MAX,
};
#define ACPI_GENL_ATTR_MAX (__ACPI_GENL_ATTR_MAX - 1)

enum {
    ACPI_GENL_CMD_UNSPEC,
    ACPI_GENL_CMD_EVENT,
    __ACPI_GENL_CMD_MAX,
};
#define ACPI_GENL_CMD_MAX (__ACPI_GENL_CMD_MAX - 1)

#define ACPI_GENL_VERSION 0x01

KToshibaNetlinkEvents::KToshibaNetlinkEvents(QObject *parent)
    : QObject(parent),
      m_notifier(NULL),
      m_socket(-1)
{
}

KToshibaNetlinkEvents::~KToshibaNetlinkEvents()
{
    if (m_notifier->isEnabled()) {
        m_notifier->setEnabled(false);
        delete m_notifier; m_notifier = NULL;
    }
    if (m_socket)
        ::close(m_socket);
}

void KToshibaNetlinkEvents::setDeviceHID(QString hid)
{
    if (hid.isEmpty() || hid.isNull())
        qWarning() << "Device HID is not valid, TVAP events monitoring will not be possible";

    m_deviceHID = hid;
}

void KToshibaNetlinkEvents::parseEvents(int socket)
{
    if (socket < 0)
        return;

    ssize_t len = ::recv(m_socket, m_eventBuffer, 1024, 0);
    if (len <= 0) {
        qCritical() << "Could not receive netlink data:" << strerror(errno);

        return;
    }

    qDebug() << "Data received from socket:" << socket;

    m_genlmsghdr = (struct genlmsghdr *)(m_eventBuffer + NLMSG_HDRLEN);
    if ((m_genlmsghdr->cmd != ACPI_GENL_CMD_EVENT) || (m_genlmsghdr->version != ACPI_GENL_VERSION)) {
        qDebug() << "Not an ACPI netlink event";

        return;
    }

    ssize_t attroffset = NLMSG_HDRLEN + GENL_HDRLEN;
    while (attroffset < len) {
        m_nlattrhdr = (struct nlattr *)(m_eventBuffer + attroffset);
        if ((m_nlattrhdr->nla_type != ACPI_GENL_ATTR_EVENT)
            || (NLA_ALIGN(m_nlattrhdr->nla_len) != NLA_ALIGN(NLA_HDRLEN
                    + sizeof(struct acpi_genl_event)))) {
            qDebug() << "ACPI netlink event not valid";

            return;
        }

        qDebug() << "Valid ACPI netlink event";
        m_event = (struct acpi_genl_event *)(m_eventBuffer + attroffset + NLA_HDRLEN);

        qDebug() << "Class:" << m_event->device_class << "Bus:" << m_event->bus_id
                 << "Type:" << hex << m_event->type << "Data:" << m_event->data;

        if (QString(m_event->bus_id) == TOSHIBA_HAPS) {
            qDebug() << "HAPS event detected";
            emit hapsEvent(m_event->type);
        } else if (QString(m_event->bus_id) == m_deviceHID) {
            qDebug() << "TVAP event detected";
            emit tvapEvent(m_event->type);
        }

        attroffset += NLA_ALIGN(m_nlattrhdr->nla_len);
    }
}

bool KToshibaNetlinkEvents::attach()
{
    memset(&m_nl, 0, sizeof(struct sockaddr_nl));
    m_nl.nl_family = AF_NETLINK;
    m_nl.nl_pid = QCoreApplication::applicationPid();
    m_nl.nl_groups = 0;
    const int buffer = 16 * 1024 * 1024;

    m_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (m_socket < 0) {
        qCritical() << "Could not open netlink socket:" << strerror(errno);

        return false;
    }

    setsockopt(m_socket, SOL_SOCKET, SO_RCVBUFFORCE, &buffer, sizeof(struct sockaddr_nl));

    if (::bind(m_socket, (struct sockaddr *) &m_nl, sizeof(struct sockaddr_nl)) < 0) {
        qCritical() << "Could not bind to netlink socket:" << strerror(errno);
        ::close(m_socket);

        return false;
    }
    qDebug() << "Binded to socket" << m_socket << "for events monitoring";

    m_notifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);

    return connect(m_notifier, SIGNAL(activated(int)), this, SLOT(parseEvents(int)));
}

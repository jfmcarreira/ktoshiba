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

#include <QSocketNotifier>

#include <KDebug>

extern "C" {
#include <sys/socket.h>
#include <linux/genetlink.h>

#include <unistd.h>
#include <errno.h>
}

#include "ktoshibanetlinkevents.h"

#define TOSHIBA_HAPS   "TOS620A:00"

/*
 * The following structs and declarations were taken from
 * linux ACPI event.c
 */
typedef char acpi_device_class[20]; 

struct acpi_genl_event {
    acpi_device_class device_class;
    char bus_id[15];
    unsigned int type;
    unsigned int data;
};

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

#define ACPI_GENL_FAMILY_NAME		"acpi_event"
#define ACPI_GENL_VERSION		0x01
#define ACPI_GENL_MCAST_GROUP_NAME	"acpi_mc_group"

KToshibaNetlinkEvents::KToshibaNetlinkEvents(QObject *parent)
    : QObject( parent ),
      m_notifier( NULL ),
      m_socket( 0 )
{
}

KToshibaNetlinkEvents::~KToshibaNetlinkEvents()
{
    detach();
}

void KToshibaNetlinkEvents::setDeviceHID(QString hid)
{
    m_deviceHID = hid;
}

void KToshibaNetlinkEvents::parseEvents(int socket)
{
    if (socket < 0)
        return;

    char event_buf[1024];
    ssize_t len;
    ssize_t attroffset;
    struct genlmsghdr *genlmsghdr = NULL;
    struct nlattr *nlattrhdr = NULL;
    struct acpi_genl_event *event = NULL;

    len = ::recv(socket, &event_buf, sizeof(event_buf), 0);
    if (len <= 0)
        return;

    kDebug() << "Data received from socket:" << socket;

    genlmsghdr = (struct genlmsghdr *)(event_buf + NLMSG_HDRLEN);
    if ((genlmsghdr->cmd != ACPI_GENL_CMD_EVENT) || (genlmsghdr->version != ACPI_GENL_VERSION)) {
        kDebug() << "Event not valid";

        return;
    }

    attroffset = NLMSG_HDRLEN + GENL_HDRLEN;
    if (attroffset < len) {
        nlattrhdr = (struct nlattr *)(event_buf + attroffset);
        if ((nlattrhdr->nla_type != ACPI_GENL_ATTR_EVENT)
                || (NLA_ALIGN(nlattrhdr->nla_len) != NLA_ALIGN(NLA_HDRLEN
                    + sizeof(struct acpi_genl_event)))) {
            kDebug() << "Event not valid";

            return;
        } else {
            kDebug() << "Valid event";
            event = (struct acpi_genl_event *)(event_buf + attroffset + NLA_HDRLEN);
        }
        attroffset += NLA_ALIGN(nlattrhdr->nla_len);
    }

    if (attroffset < len)
        kDebug() << "Multiple ACPI events detected";

    kDebug() << "Class:" << event->device_class << "Bus:" << event->bus_id
             << "Type:" << hex << event->type << "Data:" << event->data;

    if (QString(event->bus_id) == TOSHIBA_HAPS) {
        kDebug() << "HAPS event detected";
        emit hapsEvent(event->type);
    } else if (QString(event->bus_id) == m_deviceHID) {
        kDebug() << "TVAP event detected";
        emit tvapEvent(event->type);
    }
}

bool KToshibaNetlinkEvents::attach()
{
    struct sockaddr_nl nl;
    memset(&nl, 0, sizeof(nl));
    nl.nl_family = AF_NETLINK;
    nl.nl_pid = 0;
    nl.nl_groups = 0;

    m_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (m_socket < 0) {
        kError() << "Cannot open netlink socket:" << strerror(errno);

        return false;
    }

    if (::bind(m_socket, (struct sockaddr *) &nl, sizeof(nl)) < 0) {
        kError() << "Netlink socket bind failed:" << strerror(errno);
        ::close(m_socket);

        return false;
    }
    kDebug() << "Binded to socket" << m_socket << "for events monitoring";

    m_notifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);

    connect( m_notifier, SIGNAL( activated(int) ), this, SLOT( parseEvents(int) ) );

    return true;
}

void KToshibaNetlinkEvents::detach()
{
    if (m_notifier) {
        m_notifier->setEnabled(false);
        delete m_notifier; m_notifier = NULL;
    }
    if (m_socket)
        ::close(m_socket);
    m_socket = -1;
}


#include "ktoshibanetlinkevents.moc"

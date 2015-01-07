/*
   Copyright (C) 2014-2015  Azael Avalos <coproscefalo@gmail.com>

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

#include <QtCore/QSocketNotifier>
#include <QtCore/qmath.h>

#include <KDebug>

extern "C" {
#include <sys/socket.h>
#include <linux/genetlink.h>

#include <unistd.h>
#include <errno.h>
}

#include "ktoshibahddprotect.h"
#include "helperactions.h"

#define TOSHIBA_HAPS   "TOS620A:00"
#define HDD_VIBRATED   0x80
#define HDD_STABILIZED 0x81

/*
 * The following structs and declarations
 * were taken from hal.
 */
typedef char acpi_device_class[20]; 
struct acpi_genl_event {
    acpi_device_class device_class;
    char bus_id[15];
    unsigned int type;
    unsigned int data;
};

#define ACPI_GENL_VERSION 0x01

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

KToshibaHDDProtect::KToshibaHDDProtect(QObject *parent)
    : QObject( parent ),
      m_helper( new HelperActions( this ) )
{
    m_fd = hddEventSocket();
    if (m_fd >= 0)
        m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
}

KToshibaHDDProtect::~KToshibaHDDProtect()
{
    if (m_fd >= 0)
        ::close(m_fd);

    delete m_helper; m_helper = NULL;
}

void KToshibaHDDProtect::setHDDProtection(bool enabled)
{
    if (enabled)
        connect( m_notifier, SIGNAL( activated(int) ), this, SLOT( protectHDD(int) ) );
    else
        m_notifier->disconnect();
}

void KToshibaHDDProtect::unloadHDDHeads(int event)
{
    if (event == HDD_VIBRATED) {
        kDebug() << "Vibration detected";
        m_helper->unloadHeads(5000);
        emit movementDetected();
    } else if (event == HDD_STABILIZED) {
        kDebug() << "Vibration stabilized";
        m_helper->unloadHeads(0);
    }
}

void KToshibaHDDProtect::protectHDD(int socket)
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
        unloadHDDHeads(event->type);
    }
}

int KToshibaHDDProtect::hddEventSocket()
{
    struct sockaddr_nl nl;
    const int bufsize = 16 * 1024 * 1024;
    memset(&nl, 0, sizeof(struct sockaddr_nl));
    nl.nl_family = AF_NETLINK;
    nl.nl_pid = ::getpid();
    nl.nl_groups = 0xffffffff;

    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0) {
        kError() << "Cannot open netlink socket, HDD monitoring will not be possible";

        return -1;
    }

    setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &bufsize, sizeof(bufsize));

    int ret = ::bind(fd, (struct sockaddr *) &nl, sizeof(struct sockaddr_nl));
    if (ret < 0) {
        kError() << "Netlink socket bind failed, HDD monitoring will not be possible";
        ::close(fd);

        return -1;
    }

    kDebug() << "Binded to socket" << fd << "for HDD monitoring";

    return fd;
}


#include "ktoshibahddprotect.moc"

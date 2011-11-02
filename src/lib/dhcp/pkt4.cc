// Copyright (C) 2011  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <dhcp/pkt4.h>
#include <dhcp/libdhcp.h>
#include <dhcp/dhcp4.h>
#include <exceptions/exceptions.h>
#include <asiolink/io_address.h>
#include <iostream>
#include <sstream>

using namespace std;
using namespace isc::dhcp;
using namespace isc::asiolink;

namespace isc {
namespace dhcp {

const IOAddress DEFAULT_ADDRESS("0.0.0.0");

Pkt4::Pkt4(uint8_t msg_type, uint32_t transid)
     :local_addr_(DEFAULT_ADDRESS),
      remote_addr_(DEFAULT_ADDRESS),
      iface_(""),
      ifindex_(0),
      local_port_(DHCP4_SERVER_PORT),
      remote_port_(DHCP4_CLIENT_PORT),
      op_(DHCPTypeToBootpType(msg_type)),
      htype_(HTYPE_ETHER),
      hlen_(0),
      hops_(0),
      transid_(transid),
      secs_(0),
      flags_(0),
      ciaddr_(DEFAULT_ADDRESS),
      yiaddr_(DEFAULT_ADDRESS),
      siaddr_(DEFAULT_ADDRESS),
      giaddr_(DEFAULT_ADDRESS),
      bufferIn_(0), // not used, this is TX packet
      bufferOut_(DHCPV4_PKT_HDR_LEN),
      msg_type_(msg_type)
{
    /// TODO: fixed fields, uncomment in ticket #1224
    memset(chaddr_, 0, MAX_CHADDR_LEN);
    memset(sname_, 0, MAX_SNAME_LEN);
    memset(file_, 0, MAX_FILE_LEN);
}

Pkt4::Pkt4(const uint8_t* data, size_t len)
     :local_addr_(DEFAULT_ADDRESS),
      remote_addr_(DEFAULT_ADDRESS),
      iface_(""),
      ifindex_(-1),
      local_port_(DHCP4_SERVER_PORT),
      remote_port_(DHCP4_CLIENT_PORT),
      /// TODO Fixed fields, uncomment in ticket #1224
      op_(BOOTREQUEST),
      transid_(0),
      secs_(0),
      flags_(0),
      ciaddr_(DEFAULT_ADDRESS),
      yiaddr_(DEFAULT_ADDRESS),
      siaddr_(DEFAULT_ADDRESS),
      giaddr_(DEFAULT_ADDRESS),
      bufferIn_(0), // not used, this is TX packet
      bufferOut_(DHCPV4_PKT_HDR_LEN),
      msg_type_(DHCPDISCOVER)
{
    if (len < DHCPV4_PKT_HDR_LEN) {
        isc_throw(OutOfRange, "Truncated DHCPv4 packet (len=" << len
                  << " received, at least 236 bytes expected.");
    }
    bufferIn_.writeData(data, len);
}

size_t
Pkt4::len() {
    size_t length = DHCPV4_PKT_HDR_LEN; // DHCPv4 header

    /// TODO: Include options here (ticket #1228)
    return (length);
}

bool
Pkt4::pack() {
    /// TODO: Implement this (ticket #1227)

    return (false);
}
bool
Pkt4::unpack() {
    /// TODO: Implement this (ticket #1226)

    return (false);
}

std::string
Pkt4::toText() {
    stringstream tmp;
    tmp << "localAddr=[" << local_addr_.toText() << "]:" << local_port_
        << " remoteAddr=[" << remote_addr_.toText()
        << "]:" << remote_port_ << endl;
    tmp << "msgtype=" << msg_type_
        << ", transid=0x" << hex << transid_ << dec
        << endl;

    return tmp.str();
}

void
Pkt4::setHWAddr(uint8_t hType, uint8_t hlen,
                const std::vector<uint8_t>& macAddr) {
    /// TODO Rewrite this once support for client-identifier option
    /// is implemented (ticket 1228?)
    if (hlen>MAX_CHADDR_LEN) {
        isc_throw(OutOfRange, "Hardware address (len=" << hlen
                  << " too long. Max " << MAX_CHADDR_LEN << " supported.");
    }
    if ( (macAddr.size() == 0) && (hlen > 0) ) {
        isc_throw(OutOfRange, "Invalid HW Address specified");
    }

    htype_ = hType;
    hlen_ = hlen;
    memset(chaddr_, 0, MAX_CHADDR_LEN);
    memcpy(chaddr_, &macAddr[0], hlen);
}

void
Pkt4::setSname(const uint8_t* sname, size_t snameLen /*= MAX_SNAME_LEN*/) {
    if (snameLen > MAX_SNAME_LEN) {
        isc_throw(OutOfRange, "sname field (len=" << snameLen
                  << ") too long, Max " << MAX_SNAME_LEN << " supported.");
    }
    memset(sname_, 0, MAX_SNAME_LEN);
    memcpy(sname_, sname, snameLen);

    // no need to store snameLen as any empty space is filled with 0s
}

void
Pkt4::setFile(const uint8_t* file, size_t fileLen /*= MAX_FILE_LEN*/) {
    if (fileLen > MAX_FILE_LEN) {
        isc_throw(OutOfRange, "file field (len=" << fileLen
                  << ") too long, Max " << MAX_FILE_LEN << " supported.");
    }
    memset(file_, 0, MAX_FILE_LEN);
    memcpy(file_, file, fileLen);

    // no need to store fileLen as any empty space is filled with 0s
}

uint8_t
Pkt4::DHCPTypeToBootpType(uint8_t dhcpType) {
    switch (dhcpType) {
    case DHCPDISCOVER:
    case DHCPREQUEST:
    case DHCPDECLINE:
    case DHCPRELEASE:
    case DHCPINFORM:
    case DHCPLEASEQUERY:
        return (BOOTREQUEST);
    case DHCPACK:
    case DHCPNAK:
    case DHCPOFFER:
    case DHCPLEASEUNASSIGNED:
    case DHCPLEASEUNKNOWN:
    case DHCPLEASEACTIVE:
        return (BOOTREPLY);
    default:
        isc_throw(OutOfRange, "Invalid message type: "
                  << static_cast<int>(dhcpType) );
    }
}

} // end of namespace isc::dhcp

} // end of namespace isc
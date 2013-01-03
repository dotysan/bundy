// Copyright (C) 2012  Internet Systems Consortium, Inc. ("ISC")
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

#include <dns/zone_checker.h>

#include <dns/name.h>
#include <dns/rdataclass.h>
#include <dns/rrclass.h>
#include <dns/rrtype.h>
#include <dns/rrset.h>
#include <dns/rrset_collection_base.h>

#include <boost/lexical_cast.hpp>

#include <string>

using boost::lexical_cast;
using std::string;

namespace isc {
namespace dns {

namespace {
// This helper class is a trivial wrapper of ZoneCheckerCallbacks, and
// remembers it if an error happens at least once.
class CallbackWrapper {
public:
    CallbackWrapper(const ZoneCheckerCallbacks& callbacks) :
        callbacks_(callbacks), has_error_(false)
    {}

    void error(const string& reason) {
        has_error_ = true;
        callbacks_.error(reason);
    }

    void warn(const string& reason) {
        callbacks_.warn(reason);
    }

    bool hasError() const { return (has_error_); }

private:
    ZoneCheckerCallbacks callbacks_;
    bool has_error_;
};

std::string
zoneText(const Name& zone_name, const RRClass& zone_class) {
    return (zone_name.toText(true) + "/" + zone_class.toText());
}

void
checkSOA(const Name& zone_name, const RRClass& zone_class,
         const RRsetCollectionBase& zone_rrsets, CallbackWrapper& callback) {
    const AbstractRRset* rrset =
        zone_rrsets.find(zone_name, RRType::SOA(), zone_class);
    size_t count = 0;
    if (rrset != NULL) {
        for (RdataIteratorPtr rit = rrset->getRdataIterator();
             !rit->isLast();
             rit->next(), ++count) {
            if (dynamic_cast<const rdata::generic::SOA*>(&rit->getCurrent()) ==
                NULL) {
                isc_throw(Unexpected, "Zone checker found bad RDATA in SOA");
            }
        }
        if (count == 0) {
            // this should be an implementation bug, not an operational error.
            isc_throw(Unexpected, "Zone checker found an empty SOA RRset");
        }
    }
    if (count != 1) {
        callback.error("zone " + zoneText(zone_name, zone_class) + ": has " +
                       lexical_cast<string>(count) + " SOA records");
    }
}

void
checkNSNames(const Name& zone_name, const RRClass& zone_class,
             const RRsetCollectionBase& zone_rrsets,
             const AbstractRRset& ns_rrset, CallbackWrapper& callbacks) {
    if (ns_rrset.getRdataCount() == 0) {
        // this should be an implementation bug, not an operational error.
        isc_throw(Unexpected, "Zone checker found an empty NS RRset");
    }

    for (RdataIteratorPtr rit = ns_rrset.getRdataIterator();
         !rit->isLast();
         rit->next()) {
        const rdata::generic::NS* ns_data =
            dynamic_cast<const rdata::generic::NS*>(&rit->getCurrent());
        if (ns_data == NULL) {
            isc_throw(Unexpected, "Zone checker found bad RDATA in NS");
        }
        const Name& ns_name = ns_data->getNSName();
        const NameComparisonResult::NameRelation reln =
            ns_name.compare(zone_name).getRelation();
        if (reln != NameComparisonResult::EQUAL &&
            reln != NameComparisonResult::SUBDOMAIN) {
            continue;
        }
        if (zone_rrsets.find(ns_name, RRType::A(), zone_class) == NULL &&
            zone_rrsets.find(ns_name, RRType::AAAA(), zone_class) == NULL) {
            callbacks.warn("zone " + zoneText(zone_name, zone_class) +
                           ": NS has no address records (A or AAAA)");
        }
    }
}

void
checkNS(const Name& zone_name, const RRClass& zone_class,
        const RRsetCollectionBase& zone_rrsets, CallbackWrapper& callbacks) {
    const AbstractRRset* rrset =
        zone_rrsets.find(zone_name, RRType::NS(), zone_class);
    if (rrset == NULL) {
        callbacks.error("zone " + zoneText(zone_name, zone_class) +
                        ": has no NS records");
        return;
    }
    checkNSNames(zone_name, zone_class, zone_rrsets, *rrset, callbacks);
}
}

bool
checkZone(const Name& zone_name, const RRClass& zone_class,
          const RRsetCollectionBase& zone_rrsets,
          const ZoneCheckerCallbacks& callbacks) {
    CallbackWrapper my_callbacks(callbacks);

    checkSOA(zone_name, zone_class, zone_rrsets, my_callbacks);
    checkNS(zone_name, zone_class, zone_rrsets, my_callbacks);

    return (!my_callbacks.hasError());
}

} // end namespace dns
} // end namespace isc

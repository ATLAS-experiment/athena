#ifndef MUONCALIBSTREAMCNVSVCBASE_MUONCALIBSTREAMADDRESSPROVIDERSVC_H
#define MUONCALIBSTREAMCNVSVCBASE_MUONCALIBSTREAMADDRESSPROVIDERSVC_H

#include <map>
#include <set>

#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/IAddressProvider.h"
#include "GaudiKernel/ClassID.h"
#include "MuonCalibStreamCnvSvc/IMuonCalibStreamDataProviderSvc.h"

class MuonCalibStreamAddressProviderSvc : public extends<AthService, IAddressProvider> {

public:
    MuonCalibStreamAddressProviderSvc(const std::string &name, ISvcLocator *svcloc);
    virtual ~MuonCalibStreamAddressProviderSvc();

    // Service initialize
    virtual StatusCode initialize();

    // IAddressProvider interface.
    // preload the address
    virtual StatusCode preLoadAddresses(StoreID::type id, tadList &tlist);

    /// update an existing transient Address
    virtual StatusCode updateAddress(StoreID::type tp, SG::TransientAddress *tad, const EventContext &);

private:
    // type and name of the objects to create the address for.
    std::vector<std::string> m_typeNames;
    IMuonCalibStreamDataProviderSvc *m_dataSvc;
    std::map<CLID, std::set<std::string> > m_clidKey;
};
#endif

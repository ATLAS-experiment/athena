/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ALineInjectTestAlg.h"

#include <AthenaBaseComps/AthCondAlgorithm.h>
#include <AthenaKernel/IOVInfiniteRange.h>
#include <GaudiKernel/SystemOfUnits.h>
ALineInjectTestAlg::ALineInjectTestAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthCondAlgorithm{name, pSvcLocator} {}

StatusCode ALineInjectTestAlg::initialize() {
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}
StatusCode ALineInjectTestAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<ALineContainer> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()){
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(IOVInfiniteRange::infiniteTime());
    std::unique_ptr<ALineContainer> writeCdo = std::make_unique<ALineContainer>();
    
    unsigned int counter{0};
    /// Distort all Mdt stations
    {
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        for (auto itr = idHelper.module_begin();
                  itr != idHelper.module_end(); ++itr){
            ALinePar aline{};
            aline.setIdentifier(*itr);
            aline.setParameters((counter+0)*Gaudi::Units::mm,
                                (counter+1)*Gaudi::Units::mm,
                                (counter+2)*Gaudi::Units::mm,
                                (counter+3)*Gaudi::Units::deg,
                                (counter+4)*Gaudi::Units::deg,
                                (counter+5)*Gaudi::Units::deg);
            ATH_MSG_INFO("Add "<<aline<<" to move "<<m_idHelperSvc->toStringChamber(*itr));
            writeCdo->insert(std::move(aline));
            counter+=6;
           
        }
    }

    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    return StatusCode::SUCCESS;
}
  
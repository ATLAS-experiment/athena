/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMDTCNVTOOLS_MUONMDTRAWDATAPROVIDERTOOL_H
#define MUONMDTCNVTOOLS_MUONMDTRAWDATAPROVIDERTOOL_H

#include <set>

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MdtROD_Decoder.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/MdtCsm_Cache.h"
#include "StoreGate/ReadCondHandleKey.h"

class MdtCsmContainer;

namespace Muon {

    /** @class MDT_RawDataProviderTool
        @author  Mark Owen <markowen@cern.ch>
    */

    class MDT_RawDataProviderTool : public extends<AthAlgTool, IMuonRawDataProviderTool> {
    public:
        using base_class::base_class;

        /** default destructor */
        virtual ~MDT_RawDataProviderTool() = default;

        /** standard Athena-Algorithm method */
        virtual StatusCode initialize() override;

        /** IMuonRawDataProviderTool interface - EventContext-based methods */
        using IMuonRawDataProviderTool::convert;
        virtual StatusCode convert(const EventContext& ctx) const override;
        virtual StatusCode convert(const std::vector<IdentifierHash>& HashVec, 
                                   const EventContext& ctx) const override;
        virtual StatusCode convert(const std::vector<uint32_t>& robIds,
                                   const EventContext& ctx) const override;
        /** Convert method */
        virtual StatusCode convertIntoContainer(const ROBFragmentList& vecRobs,
                                                const EventContext& ctx) const;

    private:
        
        ToolHandle<MdtROD_Decoder> m_decoder{this, "Decoder", "MdtROD_Decoder/MdtROD_Decoder"};
        SG::WriteHandleKey<MdtCsmContainer> m_rdoContainerKey{this, "RdoLocation", "MDTCSM",
                                                              "Name of the MDTCSM produced by RawDataProvider"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        unsigned int m_maxhashtoUse = 0U;

        // Rob Data Provider handle
        ServiceHandle<IROBDataProviderSvc> m_robDataProvider{this, "ROBDataProviderSvc", "ROBDataProviderSvc"};

        SG::ReadCondHandleKey<MuonMDT_CablingMap> m_readKey{this, "ReadKey", "MuonMDT_CablingMap", "Key of MuonMDT_CablingMap"};
        /// This is the key for the cache for the CSM containers, can be empty
        SG::UpdateHandleKey<MdtCsm_Cache> m_rdoContainerCacheKey{this, "CsmContainerCacheKey", "",
            "Optional external cache for the CSM container"
        };
    };
}  // namespace Muon

#endif

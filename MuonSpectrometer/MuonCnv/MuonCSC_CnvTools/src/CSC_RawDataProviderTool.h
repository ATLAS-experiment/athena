/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCSC_CNVTOOLS_CSC_RAWDATAPROVIDERTOOL_H
#define MUONCSC_CNVTOOLS_CSC_RAWDATAPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "CSC_Hid2RESrcID.h"
#include "CSCcabling/CSCcablingSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCSC_CnvTools/ICSC_ROD_Decoder.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/CscRawDataCollection_Cache.h"
#include "MuonRDO/CscRawDataContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

namespace Muon {

    class CSC_RawDataProviderTool : public extends<AthAlgTool, IMuonRawDataProviderTool> {
    public:
        using base_class::base_class;

        /** default destructor */
        virtual ~CSC_RawDataProviderTool();

        /** standard Athena-Algorithm method */
        virtual StatusCode initialize() override;

        // IMuonRawDataProviderTool interface - EventContext-based methods
        using IMuonRawDataProviderTool::convert;
        virtual StatusCode convert(const EventContext& ctx) const override;
        virtual StatusCode convert(const ROBFragmentList& vecRobs, const EventContext& ctx) const override;
        virtual StatusCode convert(const std::vector<IdentifierHash>& rdoIdhVect, const EventContext& ctx) const override;

    private:
        /** function to decode the passed ROB fragments into the passed container */
        StatusCode convertIntoContainer(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
                                        const EventContext& ctx, CscRawDataContainer& container) const;

        /** member variables for algorithm properties: */
        ToolHandle<ICSC_ROD_Decoder> m_decoder{this, "Decoder", "Muon::CscROD_Decoder/CscROD_Decoder"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::WriteHandleKey<CscRawDataContainer> m_containerKey{this, "RdoLocation", "CSCRDO",
                                                               "Name of the CSCRDO produced by RawDataProvider"};
        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo", ""};
        CSC_Hid2RESrcID m_hid2re;

        ServiceHandle<IROBDataProviderSvc> m_robDataProvider{this, "RobProviderSvc", "ROBDataProviderSvc"};
        ServiceHandle<CSCcablingSvc> m_cabling{this, "CablingSvc", "CSCcablingSvc"};

        /// CSC container cache key
        SG::UpdateHandleKey<CscRawDataCollection_Cache> m_rdoContainerCacheKey{this, "CscContainerCacheKey", "",
                                                                                 "Optional external cache for the CSC container"};
    };
}  // namespace Muon

#endif

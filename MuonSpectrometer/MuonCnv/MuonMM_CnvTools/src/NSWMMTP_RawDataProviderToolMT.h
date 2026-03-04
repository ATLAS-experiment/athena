/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONMM_CNVTOOLS_NSWMMTP_RAWDATAPROVIDERTOOLMT_H
#define MUONMM_CNVTOOLS_NSWMMTP_RAWDATAPROVIDERTOOLMT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "xAODMuonRDO/NSWMMTPRDOContainer.h"
#include "MuonMM_CnvTools/INSWMMTP_ROD_Decoder.h"

namespace Muon {

  class NSWMMTP_RawDataProviderToolMT :  public extends<AthAlgTool, IMuonRawDataProviderTool> {
  public:
   
    using base_class::base_class;
    virtual ~NSWMMTP_RawDataProviderToolMT() = default;

    StatusCode initialize() override;

   
    // implemented
    using IMuonRawDataProviderTool::convert;
    StatusCode convert(const EventContext& ctx) const override;

    StatusCode convert(const std::vector<IdentifierHash>&, const EventContext&) const override;
    StatusCode convert(const std::vector<uint32_t>&, const EventContext&) const override;

  private:
    StatusCode convertFragments(const ROBFragmentList& fragments, const EventContext& ctx) const;
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    ToolHandle<INSWMMTP_ROD_Decoder>      m_decoder     {this, "Decoder", "Muon::NSWMMTP_ROD_Decoder/NSWMMTP_ROD_Decoder"};
    // Rob Data Provider handle
    ServiceHandle<IROBDataProviderSvc> m_robDataProvider{this, "ROBDataProviderSvc", "ROBDataProviderSvc"};

    SG::WriteHandleKey<xAOD::NSWMMTPRDOContainer> m_rdoContainerKey{this, "RdoLocation", "", "NSWMMTPRDOContainer"};
  };

}  // namespace Muon

#endif

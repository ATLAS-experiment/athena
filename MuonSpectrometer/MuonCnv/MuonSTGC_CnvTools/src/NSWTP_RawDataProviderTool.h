/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSTGC_CNVTOOLS_NSWTP_RawDataProviderTool_H
#define MUONSTGC_CNVTOOLS_NSWTP_RawDataProviderTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "xAODMuonRDO/NSWTPRDOContainer.h"
#include "MuonSTGC_CnvTools/INSWTP_ROD_Decoder.h"

namespace Muon {

class NSWTP_RawDataProviderTool : public extends<AthAlgTool, IMuonRawDataProviderTool> {
 public:
  
  
  using base_class::base_class;

  virtual ~NSWTP_RawDataProviderTool() = default;

  StatusCode initialize() override;

  // implemented

  StatusCode convert(const std::vector<IdentifierHash>& chamberHashes, 
                     const EventContext& ctx) const override;
  StatusCode convert(const std::vector<uint32_t>& robIDS, 
                     const EventContext& ctx) const override;
  StatusCode convert(const EventContext& ctx) const override;

 private:

  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  ToolHandle<INSWTP_ROD_Decoder>      m_decoder{this, "Decoder", "Muon::NSWTP_ROD_Decoder/NSWTP_ROD_Decoder"};
  ServiceHandle<IROBDataProviderSvc>  m_robDataProvider{this, "RobProviderSvc", "ROBDataProviderSvc"};
  SG::WriteHandleKey<xAOD::NSWTPRDOContainer> m_rdoContainerKey{this, "RdoLocation", "", "Name of of the RDO container to write to"};
};

}  // namespace Muon

#endif  // MUONSTGC_CNVTOOLS_NSWTP_RawDataProviderTool_H

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSTGC_CNVTOOLS_PadTrig_RawDataProviderTool_H
#define MUONSTGC_CNVTOOLS_PadTrig_RawDataProviderTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonRDO/NSW_PadTriggerDataContainer.h"
#include "MuonSTGC_CnvTools/IPadTrig_ROD_Decoder.h"

namespace Muon {

class PadTrig_RawDataProviderTool : public extends<AthAlgTool, IMuonRawDataProviderTool> 
{
 public:
  
  
  using base_class::base_class;
  virtual ~PadTrig_RawDataProviderTool() = default;

  StatusCode initialize() override;

  // implemented
  
  StatusCode convert(const EventContext& ctx) const override;

  StatusCode convert(const std::vector<IdentifierHash>& chamberHashes, 
                     const EventContext& ctx) const override;
  StatusCode convert(const std::vector<uint32_t>& robIDS, 
                     const EventContext& ctx) const override;
 private:
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  ToolHandle<IPadTrig_ROD_Decoder>      m_decoder{this, "Decoder", "Muon::PadTrig_ROD_Decoder/PadTrig_ROD_Decoder"};
   /** Rob Data Provider handle */
  ServiceHandle<IROBDataProviderSvc>  m_robDataProvider{this, "RobProviderSvc", "ROBDataProviderSvc"};
  SG::WriteHandleKey<NSW_PadTriggerDataContainer> m_rdoContainerKey{this, "RdoLocation", "NSW_PadTrigger_RDO", "Name of of the RDO container to write to"};
  
  unsigned int m_maxhashtoUse{0};
};

}  // namespace Muon

#endif  // MUONSTGC_CNVTOOLS_PadTrig_RawDataProviderTool_H

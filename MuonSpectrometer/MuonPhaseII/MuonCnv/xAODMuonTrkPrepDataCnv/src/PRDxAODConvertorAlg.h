/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/  

#ifndef PRDXAODCONVERTORALG_H
#define PRDXAODCONVERTORALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonPrepRawData/MdtPrepDataContainer.h"
#include "MuonPrepRawData/RpcPrepDataContainer.h"
#include "MuonPrepRawData/TgcPrepDataContainer.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"

// xAOD
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "xAODMuonPrepData/TgcStripContainer.h"
#include "xAODMuonPrepData/MMClusterContainer.h"
#include "xAODMuonPrepData/sTgcStripContainer.h"
#include "xAODMuonPrepData/sTgcWireContainer.h"
#include "xAODMuonPrepData/sTgcPadContainer.h"


namespace Muon {
/** Algorithm which converts PrepRawData to xAOD::PrepRawData
 * Not really Muon specific, but InDet already has separate convertors.
 */
class PRDxAODConvertorAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  
  virtual ~PRDxAODConvertorAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 protected:
  // Muon PRDs
  // (Could be extended to ID, but they have a pre-existing converter)

  SG::ReadHandleKey<Muon::MdtPrepDataContainer>       m_mdtPrepRawDataKey   {this, "MdtPrepRawDataKey", "MDT_DriftCircles", "Key for MDT PRD Container"};
  SG::ReadHandleKey<Muon::RpcPrepDataContainer>       m_rpcPrepRawDataKey   {this, "RpcPrepRawDataKey", "RPC_Measurements", "Key for RPC PRD Container"};
  SG::ReadHandleKey<Muon::TgcPrepDataContainer>       m_tgcPrepRawDataKey   {this, "TgcPrepRawDataKey", "TGC_MeasurementsAllBCs", "Key for TGC PRD Container"};
  SG::ReadHandleKey<Muon::MMPrepDataContainer>        m_mmPrepRawDataKey    {this, "MMPrepRawDataKey", "MM_Measurements", "Key for MM PRD Container"};
  SG::ReadHandleKey<Muon::sTgcPrepDataContainer>      m_stgcPrepRawDataKey  {this, "sTgcPrepRawDataKey", "STGC_Measurements", "Key for sTGC PRD Container"};
  
  SG::WriteHandleKey<xAOD::MdtDriftCircleContainer>   m_mdtxAODKey  {this, "MdtxAODKey", "MDT_PrepData"};
  SG::WriteHandleKey<xAOD::RpcStripContainer>         m_rpcxAODKey  {this, "RpcxAODKey", "RPC_PrepData"};
  SG::WriteHandleKey<xAOD::TgcStripContainer>         m_tgcxAODKey  {this, "TgcxAODKey", "TGC_PrepData"};
  SG::WriteHandleKey<xAOD::MMClusterContainer>        m_mmxAODKey   {this, "MMxAODKey", "MM_PrepData"};
  SG::WriteHandleKey<xAOD::sTgcStripContainer>        m_stgcStripxAODKey {this, "sTgcStripxAODKey", "sTGC_StripPrepData"};
  SG::WriteHandleKey<xAOD::sTgcWireContainer>         m_stgcWirexAODKey {this, "sTgcWirexAODKey", "sTGC_WirePrepData"};
  SG::WriteHandleKey<xAOD::sTgcPadContainer>          m_stgcPadxAODKey {this, "sTgcPadxAODKey", "sTGC_PadPrepData"};

  private:

  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  template <class INTYPE, class OUTTYPE, class OUTTYPEAUX>
  StatusCode getAndFillContainer(const SG::ReadHandleKey<INTYPE> &inKey,
                                 const SG::WriteHandleKey<OUTTYPE> &outKey, const EventContext&) const;

  template<class PRD, class xPRD>
  bool fillxPRD(const PRD& prd, xPRD& xprd) const;
};
}  // namespace ActsTrk


#endif

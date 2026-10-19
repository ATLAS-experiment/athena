/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 *   */

#ifndef TRACKOVERLAYREC_TRACKOVERLAYROISIRDOMERGERALG_H
#define TRACKOVERLAYREC_TRACKOVERLAYROISIRDOMERGERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "IRegionSelector/IRegSelTool.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

#include <atomic>

class PixelID;
class SCT_ID;

namespace TrackOverlay {

/** Builds the Pixel/SCT RDO containers used by the track overlay primary pass when
 *  RoI-limited silicon overlay is enabled.
 *
 *  For detector elements inside RoIs around EM calo clusters (ET > MinClusterEt) the fully
 *  merged signal + pileup RDO collections are taken, everywhere else the signal-only ones.
 *  Clusterization, seeding, pattern recognition and ambiguity solving downstream then see the
 *  pileup environment inside the RoIs, as in MC overlay, at a fraction of the CPU cost.
 */
class TrackOverlayRoISiRDOMergerAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

private:
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clusterKey{this, "CaloClusterKey", "egammaTopoClusters", "EM calo clusters seeding the RoIs"};

  SG::ReadHandleKey<PixelRDO_Container> m_sigPixelKey{this, "SignalPixelRDOKey", "Sig_PixelRDOs", "Signal-only Pixel RDOs"};
  SG::ReadHandleKey<PixelRDO_Container> m_fullPixelKey{this, "MergedPixelRDOKey", "PixelRDOs", "Signal + pileup Pixel RDOs"};
  SG::WriteHandleKey<PixelRDO_Container> m_outPixelKey{this, "OutputPixelRDOKey", "RoI_PixelRDOs", "Output Pixel RDOs"};

  SG::ReadHandleKey<SCT_RDO_Container> m_sigSCTKey{this, "SignalSCT_RDOKey", "Sig_SCT_RDOs", "Signal-only SCT RDOs"};
  SG::ReadHandleKey<SCT_RDO_Container> m_fullSCTKey{this, "MergedSCT_RDOKey", "SCT_RDOs", "Signal + pileup SCT RDOs"};
  SG::WriteHandleKey<SCT_RDO_Container> m_outSCTKey{this, "OutputSCT_RDOKey", "RoI_SCT_RDOs", "Output SCT RDOs"};

  ToolHandle<IRegSelTool> m_regSelPixel{this, "RegSelTool_Pixel", "", "Region selector for Pixel"};
  ToolHandle<IRegSelTool> m_regSelSCT{this, "RegSelTool_SCT", "", "Region selector for SCT"};

  Gaudi::Property<float> m_minClusterEt{this, "MinClusterEt", 10000., "Minimum cluster ET [MeV] to open an RoI"};
  Gaudi::Property<float> m_etaHalfWidth{this, "EtaHalfWidth", 0.15, "RoI half width in eta"};
  Gaudi::Property<float> m_phiHalfWidth{this, "PhiHalfWidth", 0.3, "RoI half width in phi"};
  Gaudi::Property<float> m_zHalfWidth{this, "ZHalfWidth", 225., "RoI half width in z along the beam line [mm]"};

  const PixelID* m_pixelID{nullptr};
  const SCT_ID* m_sctID{nullptr};

  mutable std::atomic<unsigned long> m_nEvents{0};
  mutable std::atomic<unsigned long> m_nRoIs{0};
  mutable std::atomic<unsigned long> m_nPixelRoIModules{0};
  mutable std::atomic<unsigned long> m_nSCTRoIModules{0};
};

} // namespace TrackOverlay

#endif

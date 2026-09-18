/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// **********************************************************************
// ITkAlignMonResidualsAlg.h
// AUTHORS: Beate Heinemann, Tobias Golling, Ben Cooper, John Alison, Pierfrancesco Butti
// Adapted to AthenaMT 2021-2022 by Per Johansson
// Ported to ITk 2026 by Makayla Vessella (TRT removed, ITk numerology)
// **********************************************************************

#ifndef ITkAlignMonResidualsAlg_H
#define ITkAlignMonResidualsAlg_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkToolInterfaces/IUpdator.h"
#include "TrkExInterfaces/IPropagator.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "InDetAlignGenTools/IInDetAlignHitQualSelTool.h"
#include "TrkTrack/TrackCollection.h"

#include <memory>
#include <string>
#include <vector>

class AtlasDetectorID;
class PixelID;
class SCT_ID;   // NB: the ITk strip identifier helper is still class SCT_ID

namespace Trk {
  class Track;
  class TrackStateOnSurface;
}

class EventContext;

class ITkAlignMonResidualsAlg :  public AthMonitorAlgorithm {

 public:

  ITkAlignMonResidualsAlg( const std::string & name, ISvcLocator* pSvcLocator );
  virtual ~ITkAlignMonResidualsAlg();
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

 private:
  StatusCode setupTools();

  StatusCode getSiResiduals(const Trk::Track*, const Trk::TrackStateOnSurface*, bool, double*) const;
  std::unique_ptr <Trk::TrackParameters> getUnbiasedTrackParameters(const Trk::Track*, const Trk::TrackStateOnSurface*) const;

  bool trackRequiresRefit(const Trk::Track*) const;

  //tools
  const AtlasDetectorID*                m_idHelper{};
  const PixelID*                        m_pixelID{};
  const SCT_ID*                         m_stripID{};

  SG::ReadHandleKey<TrackCollection> m_tracksKey {this,"TrackName2", "CombinedITkTracks", "track data key"};
  SG::ReadHandleKey<TrackCollection> m_tracksName {this,"TrackName","CombinedITkTracks", "track data key"};

  ToolHandle<Trk::IUpdator>             m_iUpdator;
  ToolHandle<Trk::IPropagator>          m_propagator;
  ToolHandle<Trk::IResidualPullCalculator>    m_residualPullCalculator;   //!< The residual and pull calculator tool handle
  ToolHandle<InDet::IInDetTrackSelectionTool> m_trackSelection; // baseline
  ToolHandle<IInDetAlignHitQualSelTool>  m_hitQualityTool;

  bool m_extendedPlots{};
  bool m_doHitQuality{false};
  int  m_checkrate {};
  bool m_doPulls {};
  bool m_applyTrkSel{};

  // ITk layer/disk counts (defaults: ATLAS-P2-RUN4-03).  They must
  // match the number of per-layer histogram groups booked by
  // ITkAlignMonResidualsAlgCfg.py.
  int m_nPixBlayers{5};
  int m_nStripBlayers{4};
  int m_nPixEClayers{9};
  int m_nStripEClayers{6};
  // Offsets added to modEta/modPhi per layer to build the stacked
  // "(Modified) Module Eta/Phi-ID" summary histograms.
  std::vector<int> m_pixBModEtaShift{12, 33, 51, 72, 93};
  std::vector<int> m_pixBModPhiShift{0, 16, 40, 76, 124};
  std::vector<int> m_pixECModPhiShift{0, 23, 58, 83, 120, 157, 206, 255, 316};
  std::vector<int> m_stripBModEtaShift{56, 171, 258, 317};
  std::vector<int> m_stripBModPhiShift{0, 32, 76, 136};
  // Per-disk stacking stride of the strip endcap module-phi summary:
  // modPhi + layerDisk * (gap + nMods)
  int m_stripECNmods{64};
  int m_stripECGap{10};

  std::vector<int> m_pixResidualX;
  std::vector<int> m_pixResidualX_2DProf;
  std::vector<int> m_pixResidualY;
  std::vector<int> m_pixResidualY_2DProf;
  std::vector<int> m_pixPullX;
  std::vector<int> m_pixPullY;
  std::vector<int> m_pixResidualXvsEta;
  std::vector<int> m_pixResidualYvsEta;
  std::vector<int> m_pixResidualXvsPhi;
  std::vector<int> m_pixResidualYvsPhi;
  std::vector<int> m_pixECAResidualX;
  std::vector<int> m_pixECAResidualY;
  std::vector<int> m_pixECResidualX_2DProf;
  std::vector<int> m_pixECResidualY_2DProf;
  std::vector<int> m_pixECCResidualX;
  std::vector<int> m_pixECCResidualY;
  std::vector<int> m_stripResidualX;
  std::vector<int> m_stripResidualX_2DProf;
  std::vector<int> m_strip_s0_ResidualX_2DProf;
  std::vector<int> m_strip_s1_ResidualX_2DProf;
  std::vector<int> m_stripECAResidualX_2DProf;
  std::vector<int> m_stripECA_s0_ResidualX_2DProf;
  std::vector<int> m_stripECA_s1_ResidualX_2DProf;
  std::vector<int> m_stripECCResidualX_2DProf;
  std::vector<int> m_stripECC_s0_ResidualX_2DProf;
  std::vector<int> m_stripECC_s1_ResidualX_2DProf;
  std::vector<int> m_stripPullX;
  std::vector<int> m_stripResidualXvsEta;
  std::vector<int> m_stripResidualXvsPhi;
};

#endif
